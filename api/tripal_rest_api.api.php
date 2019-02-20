<?php
/**
 * Run all queued indexing tasks.
 * This is intended to be invoked by a tripal job, as an alternative to using cron tasks
 * The function returns when all the queues are empty.
 *
 * @param int queue_n Number of queues to inspect (default=10)
 * @param int sleep The number of seconds to wait before trying to process new queue item
 */
function tripal_rest_api_run_indexing($sleep=120) {

    $finished = False;
    $q_name = 'elasticsearch_dispatcher';
    $cron_queues = array(
      'queue_elasticsearch_dispatcher',
      'queue_elasticsearch_queue_1',
      'queue_elasticsearch_queue_2',
      'queue_elasticsearch_queue_3',
      'queue_elasticsearch_queue_4',
      'queue_elasticsearch_queue_5'
    );

    while (!$finished) {

        $finished = True; // We consider we have finished unless we find some queue item later

        // Look into drupal queues to see if some indexing tasks are still there
        $q = DrupalQueue::get($q_name);
        $todo = $q->numberOfItems();
        if ($todo > 0) {
            print "Still $todo items in queue $q_name.\n";
	        foreach($cron_queues as $cron_q_name){
             // Look at the corresponding cron queue to see if it is busy or not
                $job = _ultimate_cron_job_load($cron_q_name);
                $lock_id = $job->isLocked();
                if ($lock_id === FALSE) {
                    print "Cron queue $cron_q_name is available, launching cron-run.\n";
                    // The cron-run command watchdog log can be found in the cron queue from the admin interface
                    exec("export BASE_URL=http://localhost/ && drush cron-run $cron_q_name > /dev/null 2> /dev/null &"); // TODO See if we should use drush_invoke_process()
                    sleep(1); // Wait just a sec to make sure process is really started
                }
                else {
                    print "Cron queue $q_name is busy (lock: $lock_id), skipping this one.\n";
                }
                $finished = False; // Something was just launched, we'll need to continue checking this queue
	        }
        }

        // Wait a little before trying to run new queue items
        if (!$finished) {
            print "\nSleeping for $sleep seconds.\n";
            sleep($sleep);
        }
    }

    print "\nDone.\n";
}
