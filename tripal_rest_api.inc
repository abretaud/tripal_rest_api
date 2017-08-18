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

    exec("drush trp-run-jobs --username=admin > $stdout 2> $stderr &");

    return (object)array(
            'status' => 'running',
            'stdout' => $stdout,
            'stderr' => $stderr
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
 * Callback for indexing with elasticsearch
 *
 * @param object $data
 * @return object
 */
function _tripal_rest_api_elasticsearch_index($data) {

    if ($data['mode'] == 'table') {
        // Indexing a single table

        if (!isset($data['table'])) {
            return services_error('Missing Chado table name \'table\'', 406);
        }

        $website_or_table = 'database table';
        $website_base_url = ''; // not used
        $index_name = $data['index_name'];
        $index_table = $data['table'];
        $tokenizer = $data['tokenizer'];
    }
    else {
        // Indexing the whole website
        $website_or_table = 'website';
        $website_base_url = 'http://localhost/';
        $index_name = 'website';
        $index_table = ''; // not used
        $tokenizer = ''; //not used
    }

    if (!isset($data['queues'])) {
        $data['queues'] = 10;
    }
    $queue_N = $data['queues'];

    // Add indexing tasks to the queues
    $form_state = array();
    $form_state['values']['queue_number'] = $queue_N;
    $form_state['values']['website_or_table'] = $website_or_table;
    $form_state['values']['website_base_url'] = $website_base_url;
    $form_state['values']['index_name'] = $index_name;
    if (!empty($index_table)) {
        $form_state['values']['index_table'] = $index_table;
    }
    if (!empty($tokenizer)) {
        $form_state['values']['index_settings']['tokenizer'] = $tokenizer;
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

    if ($data['mode'] == 'table') {
        // Create the search block
        $form_state = array();
        $form_state['values']['index_name'] = $index_name;
        $form_state['values']['table_name'] = $index_table;
        $form_state['values']['index_fields'] = array_keys($data['fields']);
        drupal_form_submit('table_search_interface_building_form_submit', $form_state);

        if (form_get_errors() != '') {
            return (object)array(
                'status' => 'error',
                'errors' => json_encode(form_get_errors())
            );
        }

        if (!empty($data['links'])) {
            $form_state = array();
            $form_state['values']['search_block'] = $index_name;
            $form_state['values']['field_wrapper'] = $data['links'];
            drupal_form_submit('link_results_to_pages_form', $form_state);

            if (form_get_errors() != '') {
                return (object)array(
                    'status' => 'error',
                    'errors' => json_encode(form_get_errors())
                );
            }
        }

        // Clear the cache (needed for search block to show up)
        drupal_flush_all_caches();
    }

    return $content;
}
