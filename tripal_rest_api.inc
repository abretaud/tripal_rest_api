<?php
// $Id$

/**
 * Callback for creating job resources.
 *
 * @param object $data
 * @return object
 */
function _tripal_rest_api_job_create($data) {

    global $user;

    $data = json_decode(json_encode($data), FALSE); // Convert to a PHP object instead of array

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
