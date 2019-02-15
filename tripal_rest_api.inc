<?php
/**
 * Callback for creating job resources.
 *
 * @param object $data
 * @return object
 */
function _tripal_rest_api_job_create($data) {

    global $user;

    $args = $data['arguments']; // Save this before as we want to keep it as an array

    $data = json_decode(json_encode($data), FALSE); // Convert to a PHP object instead of array

    $data->arguments = $args; // Restore the array to replace potential stdClass generated above

    $data->uid = $user->uid;

    if (!isset($data->name)) {
        return services_error('Missing job attribute \'name\'', 406);
    }

    if (!isset($data->modulename)) {
        return services_error('Missing job attribute \'modulename\'', 406);
    }

    if (!isset($data->callback)) {
        return services_error('Missing job attribute \'callback\'', 406);
    }

    if (!isset($data->arguments)) {
        return services_error('Missing job attribute \'arguments\'', 406);
    }

    $job_id = tripal_rest_api_job_create($data);
    return (object)array(
        'job_id' => $job_id,
    );
}

/**
 * Callback for creating a job for a data importer.
 *
 * @param object $data
 * @return object
 */
function _tripal_rest_api_job_create_import($data) {

    global $user;

    $args = $data['arguments']; // Save this before as we want to keep it as an array

    $data = json_decode(json_encode($data), FALSE); // Convert to a PHP object instead of array

    $data->arguments = $args; // Restore the array to replace potential stdClass generated above

    $data->uid = $user->uid;

    if (!isset($data->name)) {
        return services_error('Missing job attribute \'name\'', 406);
    }

    if (!isset($data->importer)) {
        return services_error('Missing job attribute \'importer\'', 406);
    }

    if (!isset($data->file)) {
        return services_error('Missing job attribute \'file\'', 406);
    }

    if (!isset($data->arguments)) {
        return services_error('Missing job attribute \'arguments\'', 406);
    }

    $job_id = tripal_rest_api_job_create_import($data);
    return (object)array(
        'job_id' => $job_id,
    );
}

/**
 * Callback for retrieving job resources.
 *
 * @param int $id
 * @return object
 */
function _tripal_rest_api_job_retrieve($id) {
    return tripal_rest_api_job_retrieve($id);
}

/**
 * Callback for listing jobs.
 *
 * @return array
 */
function _tripal_rest_api_job_index() {

  $jobs = db_query("SELECT * FROM {tripal_jobs} ORDER BY job_id")->fetchAll();

  foreach ($jobs as $id => $job) {
      $job->arguments = unserialize($job->arguments);
      $job->raw_arguments = $job->arguments;
      $arg_hook = $job->modulename . "_job_describe_args";
      if (is_callable($arg_hook)) {
        $new_args = call_user_func_array($arg_hook, array($job->callback, $job->arguments));
        if (is_array($new_args) and count($new_args)) {
          $job->arguments = $new_args;
        }
        else {
          $job->arguments = $job->arguments;
        }
      }
  }

  return $jobs;
}

/**
 * Callback for running all jobs in queue.
 *
 * @return object
 */
function _tripal_rest_api_job_run() {

    if (tripal_is_job_running()) {
        // Some job is already running, can't launch a new one
        return (object)array(
            'status' => 'busy',
        );
    }

    // Create 2 temp files for stdout and stderr
    $temp_dir = sys_get_temp_dir();
    $logs_dir = $temp_dir.'/tripal-job-logs/';
    if (!is_dir($logs_dir)) {
        mkdir($logs_dir, 0777, true);
    }
    $stdout = tempnam($logs_dir, 'stdout');
    $stderr = tempnam($logs_dir, 'stderr');

    // FIXME without --single this will run all jobs in queue but with --single it will return immediatly if a job is already running
    exec("drush trp-run-jobs --username=admin > $stdout 2> $stderr &"); // TODO See if we should use drush_invoke_process()

    return (object)array(
            'status' => 'running',
            'stdout' => $stdout,
            'stderr' => $stderr
        );
}

/**
 * Callback to check if some job is running
 * A side effect of this function is that it will clean the state of jobs that failed in a bad way
 * (e.g. if exit was called during the job execution)
 *
 * @return object
 */
function _tripal_rest_api_job_is_running() {

    return (object)array(
        'status' => tripal_is_job_running(),
    );
}

/**
 * Retrieve job output log.
 *
 * @return object
 */
function _tripal_rest_api_job_logs($data) {

    // Create 2 temp files for stdout and stderr
    $stdout = realpath($data['stdout']);
    $stderr = realpath($data['stderr']);
    $stdout_dir = dirname($stdout);
    $stderr_dir = dirname($stderr);
    $stdout_name = basename($stdout);
    $stderr_name = basename($stderr);

    $temp_dir = sys_get_temp_dir();
    $logs_dir = $temp_dir.'/tripal-job-logs';

    // Check many things to avoid getting unauthorized file content
    if (($stdout_dir != $stderr_dir) || ($stdout_dir != $logs_dir)
        || substr($stdout_name, 0, 6) != 'stdout' || substr($stderr_name, 0, 6) != 'stderr') {
        return (object)array(
            'status' => 'error',
            'msg' => "You do not have permission to access $stdout or $stderr, $stdout_dir, $logs_dir, $stdout_name, ".substr($stdout_name, 0, 6),
            'stdout' => json_encode(""),
            'stderr' => json_encode("")
        );
    }

    return (object)array(
            'status' => 'ok',
            'stdout' => json_encode(file_get_contents($stdout)),
            'stderr' => json_encode(file_get_contents($stderr))
        );
}

/**
 * Callback for listing chado table content
 *
 * @param object $data
 * @return object
 */
function _tripal_rest_api_chado_list($data) {

    if (!isset($data['table'])) {
        return services_error('Missing Chado table name \'table\'', 406);
    }
    $content = tripal_rest_api_chado_list($data['table']);

    return $content;
}

/**
 * Callback for listing taxonomic ranks from chado table content
 *
 * @param object $data
 * @return object
 */
function _tripal_rest_api_chado_taxonomic_ranks() {

    return tripal_rest_api_chado_taxonomic_ranks();
}

/**
 * Callback for setting up the tuning form for elasticsearch
 *
 * @param object $data
 * @return object
 */

function _tripal_rest_api_elasticsearch_tuning() {

    $form_state = array('values' => array());
    drupal_form_submit('tripal_elasticsearch_tuning_form', $form_state);
    if (form_get_errors() != '') {
        return (object)array(
            'status' => 'error',
            'errors' => json_encode(form_get_errors())
        );
    }

    return (object)array(
        'status' => 'ok',
        'msg' => 'Tuning complete'
    );
}

/**
 * Callback for indexing with elasticsearch
 *
 * @param object $data
 * @return object
 */

function _tripal_rest_api_elasticsearch_index($data) {

    // Four possibilities in Tripal 3
    // Full website entities, full website nodes, gene_search, and table

    if ($data['mode'] == 'table') {
        // Indexing a single table

        if (!isset($data['table'])) {
            return services_error('Missing Chado table name \'table\'', 406);
        }

        $index_type = 'database';
        $index_name = $data['index_name'];
        $index_table = $data['table'];
    }

    elseif ($data['mode'] == 'gene') {
        // Gene data search

        $index_type = 'gene_search';
        $index_name = 'gene_search';
    }

    elseif ($data['mode'] == 'nodes') {
        // Indexing the whole website for nodes
        $index_type = 'website';
        $index_name = 'website_nodes';
    } 
    else {
        // Indexing whole website for entities
        $index_type = 'entities';
        $index_name = 'website_entities';
        // Need to make sure the tuning is done, if not, do it.
        $count = db_query('SELECT COUNT(id) FROM {tripal_elasticsearch_priority}')->fetchField();
        if (intval($count) === 0) {
            $form_state = array('values' => array());
            drupal_form_submit('tripal_elasticsearch_tuning_form', $form_state);
            if (form_get_errors() != '') {
                return (object)array(
                    'status' => 'error',
                    'errors' => json_encode(form_get_errors())
                );
            }       
        }
    }

    // Add indexing tasks to the queues
    $form_state = array();
    $form_state['values']['index_type'] = $index_type;
    $form_state['values']['index_name'] = $index_name;

    $form_state['values']['exposed'] = isset($data['exposed']) ? $data['exposed'] : 'False';
    $form_state['values']['index_form_url'] = isset($data['index_url']) ? $data['index_url'] : '';

    if (!empty($index_table)) {
        $form_state['values']['index_table'] = $index_table;
    }
    if (isset($data['tokenizer'])) {
        $form_state['values']['index_settings']['tokenizer'] = $tokenizer;
    }
    if (isset($data['token_filters'])) {
        $form_state['values']['index_settings']['token_filters'] = $token_filters;
    }

    if (isset($data['fields'])) {
        $form_state['values']['table_fields'] = $data['fields'];
    }

    drupal_form_submit('tripal_elasticsearch_indexing_form', $form_state);

    if (form_get_errors() != '') {
        return (object)array(
            'status' => 'error',
            'errors' => json_encode(form_get_errors())
        );
    }

    return (object)array(
        'status' => 'ok',
        'msg' => 'Index complete'
    );
}

/**
 * Create a new entity.
 *
 * @return object
 */
function _tripal_rest_api_entity_create($data) {

    $allowed_types = array('Organism', 'Analysis');

    if (!isset($data['entity_type'])) {
        return services_error("'entity_type' is required", 406);
    }

    $type = $data['entity_type'];

    if (!in_array($data['entity_type'], $allowed_types)) {
        return services_error("'entity_type' must be one of '".join("', '", $allowed_types)."'", 406);
    }

    $bundle = tripal_load_bundle_entity(array('label' => $type));
    if (!$bundle) {
        return (object)array(
            'status' => 'error',
            'msg' => 'Invalid content type: ' . $type,
        );
        return services_error('Invalid content type: ' . $type, 406);
    }

    // Get the list of available fields
    $field_mapping = array();
    $fields = field_info_fields();
    foreach ($fields as $field) {
      if (array_key_exists('TripalEntity', $field['bundles'])) {
        foreach ($field['bundles']['TripalEntity'] as $bundle_name) {
          if ($bundle_name == $bundle->name) {
            $instance = field_info_instance('TripalEntity', $field['field_name'], $bundle_name);
            if (array_key_exists('term_accession', $instance['settings'])){
              $vocabulary = $instance['settings']['term_vocabulary'];
              $accession = $instance['settings']['term_accession'];
              $term = tripal_get_term_details($vocabulary, $accession);
              $key = $term['name'];
              $key = strtolower(preg_replace('/ /', '_', $key));
              $field_mapping[$key] = $field['field_name'];
            }
          }
        }
      }
    }

    $converted_params = array();
    foreach ($data['params'] as $key => $value) {
      if (!isset($field_mapping[$key])) {
        return services_error("Invalid parameter: ".$key.".", 406);
      }
      $converted_params[$field_mapping[$key]] = $value;
    }

    // Create the entity
    $ec = entity_get_controller('TripalEntity');
    $entity_data = array(
      'bundle' => $bundle->name,
      'term_id' => $bundle->term_id,
    );

    $entity = $ec->create($entity_data);
    $instances = field_info_instances('TripalEntity', $bundle->name);

    foreach ($instances as $field_name => $instance) {
      if (isset($converted_params[$field_name])) {
        $key_field = field_info_field($field_name);

        $field_class = $key_field['type'];
        if (tripal_load_include_field_class($field_class)) {
          $key_field = new $field_class($key_field, $instance);
          $elements = $key_field->elementInfo()[$key_field->getFieldTermID()];

          if (isset($elements['elements'])) {
            if ($type == 'Organism') {
                $label_term = 'rdfs:label';
                $infraspecific_name_term = tripal_get_chado_semweb_term('organism', 'infraspecific_name');
                $infraspecific_type_term = tripal_get_chado_semweb_term('organism', 'type_id');

                $label = $converted_params[$field_name]['type_id'] . ' ' . $converted_params[$field_name]['infraspecific_name'];
                $entity->{$field_name}['und'][0]['value'][$label_term] = $label;
                $entity->{$field_name}['und'][0]['value'][$infraspecific_name_term] = $converted_params[$field_name]['infraspecific_name'];
                $entity->{$field_name}['und'][0]['value'][$infraspecific_type_term] = $converted_params[$field_name]['type_id'];

                $entity->{$field_name}['und'][0]['chado-organism__type_id'] = $converted_params[$field_name]['type_id'];
                $entity->{$field_name}['und'][0]['chado-organism__infraspecific_name'] = $converted_params[$field_name]['infraspecific_name'];
            }
            elseif ($type == 'Analysis') {
                $sourcename_term = tripal_get_chado_semweb_term('analysis', 'sourcename');
                $sourceversion_term = tripal_get_chado_semweb_term('analysis', 'sourceversion');
                $sourceuri_term = tripal_get_chado_semweb_term('analysis', 'sourceuri');

                $entity->{$field_name}['und'][0]['value'][$sourcename_term] = $converted_params[$field_name]['sourcename'];
                $entity->{$field_name}['und'][0]['value'][$sourceversion_term] = $converted_params[$field_name]['sourceversion'];
                $entity->{$field_name}['und'][0]['value'][$sourceuri_term] = $converted_params[$field_name]['sourceuri'];

                $entity->{$field_name}['und'][0]['chado-analysis__sourcename'] = $converted_params[$field_name]['sourcename'];
                $entity->{$field_name}['und'][0]['chado-analysis__sourceversion'] = $converted_params[$field_name]['sourceversion'];
                $entity->{$field_name}['und'][0]['chado-analysis__sourceuri'] = $converted_params[$field_name]['sourceuri'];
            }
          }
        }
        else {
          $field_table = $instance['settings']['chado_table'];
          $field_column = $instance['settings']['chado_column'];
          $entity->{$field_name}['und'][0]['value'] = $converted_params[$field_name]; // Needed to generate the entity label
          $entity->{$field_name}['und'][0]['chado-' . $field_table . '__' . $field_column] = $converted_params[$field_name];
        }
      }
    }

    $entity = $entity->save();
    if (!$entity) {
        return services_error('Could not create entity. Maybe this entity already exists?', 500);
    }

    global $base_url;
    $service_path = $base_url . '/web-services';
    $new_entity_path = $service_path . '/content/v0.1/' . urlencode($type) . '/' . $entity->id;
    drupal_goto($new_entity_path, array('external' => TRUE), 201);
}


/**
 * List tripal entities.
 *
 * @return object
 */
function _tripal_rest_api_entity_list() {

  $bundles = db_query("SELECT * FROM {tripal_bundle} ORDER BY term_id")->fetchAll();

  $ret = array();
  foreach ($bundles as $id => $bundle) {
      $ret[$bundle->label] = $bundle->name;
  }

  return $ret;
}
