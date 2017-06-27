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
    $stdout = '/dev/null';
    $stderr = '/dev/null';
    if (!empty(getenv('GALAXY_SHARED_DIR'))) {
        $logs_dir = getenv('GALAXY_SHARED_DIR').'/logs/';
        mkdir($logs_dir, 0777, true);
        $stdout = tempnam($logs_dir, 'stdout');
        $stderr = tempnam($logs_dir, 'stderr');
    }

    exec("drush trp-run-jobs --username=admin > $stdout 2> $stderr &");

    return (object)array(
            'status' => 'running',
            'stdout' => $stdout,
            'stderr' => $stderr
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
 * Callback for indexing with elasticsearch
 *
 * @param object $data
 * @return object
 */
function _tripal_rest_api_elasticsearch_index($data) {

    if (!isset($data['table'])) {
        return services_error('Missing Chado table name \'table\'', 406);
    }

    $queue_N = $data['queues'];

    // Convert human readable table name to id
    $table_list = get_table_list();
    foreach ($table_list as $table_id => $table_name) {
        if ($data['table'] == $table_name) {
            $table_dropdown = $table_id;
            break;
        }
    }

    if (!isset($table_dropdown)) {
        return services_error('Could not find Chado table with name \''.$data['table'].'\'', 406);
    }

    if (!isset($data['queues'])) {
        $data['queues'] = 10;
    }

    $field_list = get_column_list($data['table']);

    $fields = array();
    if (!empty($data['fields'])) {
        foreach ($data['fields'] as $asked_field) {
            foreach ($field_list as $field_id => $field_name) {
                if ($asked_field == $field_name) {
                    $fields[strval($field_id)] = strval($field_id);
                    break;
                }
            }
        }
    }
    else if ($table_dropdown > 0) {
      # No field given, index all fields
      foreach ($field_list as $field_id => $field_name) {
        $fields[strval($field_id)] = strval($field_id);
      }
    }

    // Add indexing tasks to the queues
    $content = run_elasticsearch_indexing($table_dropdown, $queue_N, $fields);

    if ($table_dropdown > 0) {
        // Create the search block
        tripal_elasticsearch_add_block($data['table'], $data['fields']);

        // Clear the cache (needed for search block to show up)
        drupal_flush_all_caches();

        if (!empty($data['links'])) {
            tripal_elasticsearch_add_links($data['table'], $data['links']);
        }
    }

    return $content;
}
