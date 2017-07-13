<?php
/**
 * Implementation of hook_default_services_endpoint().
 */
function tripal_rest_api_default_services_endpoint() {
    $endpoints = array();

    $endpoint = new stdClass;
    $endpoint->disabled = FALSE; /* Edit this to true to make a default endpoint disabled initially */
    $endpoint->api_version = 3;
    $endpoint->name = 'tripal_rest_api';
    $endpoint->title = 'Tripal REST API';
    $endpoint->server = 'rest_server';
    $endpoint->path = 'tripal_api';
    $endpoint->authentication = array('services' => 'services');
    $endpoint->server_settings = array(
            'formatters' => array(
            'json' => TRUE,
            'bencode' => FALSE,
            'jsonp' => FALSE,
            'php' => FALSE,
            'xml' => FALSE,
        ),
            'parsers' => array(
            'application/json' => TRUE,
            'application/vnd.php.serialized' => FALSE,
            'application/x-www-form-urlencoded' => FALSE,
            'application/xml' => FALSE,
            'multipart/form-data' => FALSE,
            'text/xml' => FALSE,
        ),
    );
    $endpoint->resources = array(
        'job' => array(
            'operations' => array(
                'retrieve' => array(
                    'enabled' => '1',
                    ),
                'create' => array(
                    'enabled' => '1',
                    ),
                'index' => array(
                    'enabled' => '1',
                    ),
                ),
            'actions' => array(
                'run' => array(
                    'enabled' => '1',
                    ),
                'logs' => array(
                    'enabled' => '1',
                    ),
                ),
            ),
        'chado' => array(
            'actions' => array(
                'list' => array(
                    'enabled' => '1',
                    ),
                'taxonomic_ranks' => array(
                    'enabled' => '1',
                    )
                ),
            ),
        'elasticsearch' => array(
            'actions' => array(
                'index' => array(
                    'enabled' => '1',
                    ),
                ),
            ),
        'node' => array(
            'operations' => array(
                'retrieve' => array(
                    'enabled' => '1',
                    ),
                'create' => array(
                    'enabled' => '1',
                    ),
                'update' => array(
                    'enabled' => '1',
                    ),
                'delete' => array(
                    'enabled' => '1',
                    ),
                'index' => array(
                    'enabled' => '1',
                    ),
            ),
        ),
        'user' => array(
            'operations' => array(
                'retrieve' => array(
                    'enabled' => '0',
                ),
                'create' => array(
                    'enabled' => '0',
                ),
                'update' => array(
                    'enabled' => '0',
                ),
                'delete' => array(
                    'enabled' => '0',
                ),
                'index' => array(
                    'enabled' => '0',
                ),
            ),
            'actions' => array(
                'login' => array(
                    'enabled' => '1',
                    'settings' => array(
                        'services' => array(
                            'resource_api_version' => '1.0',
                        ),
                    ),
                ),
                'logout' => array(
                    'enabled' => '1',
                    'settings' => array(
                        'services' => array(
                            'resource_api_version' => '1.0',
                        ),
                    ),
                ),
                'token' => array(
                    'enabled' => '1',
                ),
                'request_new_password' => array(
                    'enabled' => '0',
                ),
                'user_pass_reset' => array(
                    'enabled' => '0',
                ),
                'register' => array(
                    'enabled' => '0',
                ),
            ),
            'targeted_actions' => array(
                'cancel' => array(
                    'enabled' => '0',
                ),
                'password_reset' => array(
                    'enabled' => '0',
                ),
                'resend_welcome_email' => array(
                    'enabled' => '0',
                ),
            ),
        ),
        );
    $endpoint->debug = 1;
    $endpoints[] = $endpoint;

    return $endpoints;
}
