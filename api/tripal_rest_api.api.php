<?php
/**
 * Run all queued indexing tasks.
 * This is intended to be invoked by a tripal job, as an alternative to using cron tasks
 * The function returns when all the queues are empty.
 *
 * @param int queue_n Number of queues to inspect (default=10)
 * @param int sleep The number of seconds to wait before trying to process new queue item
 */
function tripal_rest_api_run_indexing($queue_n=10, $sleep=120) {

    $queues = range(0, $queue_n - 1);

    $finished = False;

    while (!$finished) {

        $finished = True; // We consider we have finished unless we find some queue item later

        foreach($queues as $q_n){
            $q_name = 'elasticsearch_queue_'.$q_n;
            $q = DrupalQueue::get($q_name);
            $todo = $q->numberOfItems();
            if ($todo > 0) {
                print "\nStill $todo items in queue $q_name, running cron-run.\n";
                exec("export BASE_URL=http://localhost/ && drush cron-run queue_$q_name > /tmp/stdoutcronrun 2> /tmp/stdoutcronrun &");
                $finished = False; // Something was just launched, we'll need to continue checking this queue
            }
        }

        if (!$finished) {
            print "\nSleeping for $sleep seconds.\n";
            sleep($sleep); // Wait a little before trying to run new queue items
        }
    }

    print "\nDone.\n";
}
