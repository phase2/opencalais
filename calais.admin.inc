<?php
/*
  Copyright (C) 2008 by Phase2 Technology.
  Author(s): Frank Febbraro, Irakli Nadareishvili

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY. See the LICENSE.txt file for more details.

  $Id: calais.admin.inc,v 1.1.2.8.2.5 2009/12/07 08:35:43 inadarei Exp $
 */
/**
 * @file
 */

/**
 * Build the administration page for Calais and Content Types
 */
function calais_admin_settings() {
  $form = array();

  $form['calais_store_rdf'] = array(
    '#type' => 'checkbox',
    '#title' => t('Store Calais RDF Locally'),
    '#default_value' => variable_get('calais_store_rdf', FALSE),
    '#description' => t('If checked, this will store the RDF data returned from Calais in the local RDF store. This data can be used to get a deeper view into the Calais results as the functionality is more fully developed. It also allows for future semantic applications to query and build upon this linked data.'),
  );

  calais_build_entity_settings($form);
  calais_build_nodetype_settings($form);

  $form['#submit'][] = 'calais_admin_settings_submit';

  $form = system_settings_form($form);
  return $form;
}

/**
 * Build the Entity selector. Used for Entity suppression.
 */
function calais_build_entity_settings(&$form, $type = 'global', $name = 'Global', $allow_disable = FALSE) {
  $entity_attributes = array();
  $entities = array_keys(calais_get_entity_vocabularies());
  sort($entities);

  $disabled = FALSE;
  if ($allow_disable) {
    $var_name = "calais_use_global_{$type}";
    $disabled = variable_get($var_name, TRUE);

    $form[$type][$var_name] = array(
      '#type' => 'checkbox',
      '#title' => t('Use Calais Global Entity defaults'),
      '#default_value' => $disabled,
      '#description' => t('If checked, this content type will use the Global Calais Entities.'),
    );

    if ($disabled) {
      $entity_attributes['style'] = 'display:none';
    }

    $field_id = 'edit-'. str_replace(array('][', '_', ' '), '-', $var_name);
    drupal_add_js("$(document).ready(function() {
      $('#$field_id').click(function() {
        $('.{$type}_entities').toggle();
      });
    });", 'inline');
  }

  $form[$type]["calais_entity_settings_{$type}"] = array(
    '#type' => 'fieldset',
    '#title' => $name,
    '#attributes' => array_merge(array('class' => "{$type}_entities"), $entity_attributes),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form[$type]["calais_entity_settings_{$type}"]["calais_applied_entities_{$type}"] = array(
    '#type' => 'checkboxes',
    '#title' => t('Which Calais Entities do you wish to use?'),
    '#default_value' => variable_get("calais_applied_entities_{$type}", $entities),
    '#options' => drupal_map_assoc($entities),
    '#description' => t('Choose which Calais vocabularies should be included for term suggestions.'),
  );
}

/**
 * Build the node type settings form for Calais integration.
 */
function calais_build_nodetype_settings(&$form) {
  node_types_rebuild();

  $node_types = node_get_types('types', NULL, TRUE);
  $options = array(
    CALAIS_PROCESS_NO => t('Not processed by Calais (default)'),
    CALAIS_PROCESS_MANUAL => t("Suggest terms, but DON'T apply them"),
    CALAIS_PROCESS_AUTO => t('Apply all suggested terms on every update'),
    CALAIS_PROCESS_AUTO_ONCE => t('Apply all suggested terms on creation ONLY'),
  );

  foreach ($node_types as $nt) {
    $key = drupal_strtolower($nt->type);
    $name = $nt->name;
    $param_name = array('@node' => $name);
    
    $form[$key] = array(
      '#type' => 'fieldset',
      '#title' => $name,
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $processing = variable_get("calais_node_{$key}_process", CALAIS_PROCESS_NO);
    $form[$key]["calais_node_{$key}_process"] = array(
      '#type' => 'radios',
      '#parents' => array('calais_node_'. $key .'_process'),
      '#title' => t('Calais Processing'),
      '#default_value' => $processing,
      '#options' => $options,
    );
    
    _calais_build_semanticproxy_config($form, $nt);
    
    $form[$key]["calais_api_allow_searching_{$key}"] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow Calais Searching'),
      '#default_value' => variable_get("calais_api_allow_searching_{$key}", TRUE),
      '#description' => t('Indicates whether future searches can be performed on the extracted metadata by Calais'),
    );
    $form[$key]["calais_api_allow_distribution_{$key}"] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow Calais Distribution'),
      '#default_value' => variable_get("calais_api_allow_distribution_{$key}", TRUE),
      '#description' => t('Indicates whether the extracted metadata can be distributed by Calais'),
    );
    $form[$key]["calais_threshold_{$key}"] = array(
      '#type' => 'textfield',
      '#size' => 5,
      '#title' => t('Relevancy Threshold'),
      '#default_value' => variable_get("calais_threshold_{$key}", 0.0),
      '#description' => t('Determine how relevant a term must be in order for Calais to suggest it for a particular node.  Based on a 0.00-1.00 scale, with 0.00 being least relevant (i.e. many terms appear).'),
    );

    calais_build_entity_settings($form, $key, $name, TRUE);
  }
}

/**
 * Configuration for Semantic Proxy integration. This will
 */
function _calais_build_semanticproxy_config(&$form, $node_type) {
  
  $spfields = array('' => "Don't process with SemanticProxy");
  $docfields = array('' => "Don't store document text");

  // If this content type is configured as a feed item
  if(module_exists('feedapi_node')) {
    $feed_types = feedapi_get_types();
    foreach($feed_types as $type => $name) {
      $settings = feedapi_get_settings($type);
      if($settings['processors']['feedapi_node']['content_type'] == $node_type->type) {
        $spfields['calais_feedapi_node'] = t('Feed Item Original URL');
      }
    }
  }

  if(module_exists('content')) {
    $content_type = content_types($node_type->type);
    $type_url_str = $content_type['url_str'];
    $fields = $content_type['fields'];
    if(is_array($fields) && !empty($fields)) {
      foreach($fields as $name => $field) {
        // Fields to submit to SemanticProxy
        if($field['type'] == 'link' || ($field['type'] == 'text' && $field['widget']['type'] == 'text_textfield')) {
          $spfields[$name] = $field['widget']['label'] . ' (' . $field['field_name'] . ')';
        }
        // Fields to store document text
        if($field['type'] == 'text' && $field['widget']['type'] == 'text_textarea') { 
          $docfields[$name] = $field['widget']['label'] . ' (' . $field['field_name'] . ')';
        }
      }
    }
  }
  
  if(count($spfields) == 1)
    return;
    
  $key = drupal_strtolower($node_type->type);  
  $form[$key]['semanticproxy'] = array(
    '#type' => 'fieldset',
    '#title' => t('Semantic Proxy'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form[$key]['semanticproxy']["calais_semanticproxy_field_{$key}"] = array(
    '#type' => 'select',
    '#title' => t('Field to send to SemanticProxy'),
    '#options' => $spfields,
    '#default_value' => variable_get("calais_semanticproxy_field_{$key}", ''),
    '#description' => t('SemanticProxy submits the content of a URL to Calais for processing. You would use this if your content type is a node that has a short blurb/summary with the full content living on another site. This will ask SemanticProxy to get the contents of that URL and submit it to Calais for you. This field must contain a URL.'),
  );  
  $form[$key]['semanticproxy']["calais_semanticproxy_document_{$key}"] = array(
    '#type' => 'select',
    '#title' => t('Field to save document text'),
    '#options' => $docfields,
    '#default_value' => variable_get("calais_semanticproxy_document_{$key}", ''),
    '#description' => t('SemanticProxy submits the content of a URL to Calais for processing. This is the field where you would like to save the document content that sent to Calais for processing. You can use this field to submit to to other services such as yahoo Terms, etc. You could use this field for display, but chances are it is not fit for presentation. You can hide this field from display !url', array('!url' => l('here', "admin/content/node-type/{$type_url_str}/display"))),
  );  
}

/**
 * Have to override so that we can manage vocabulary - node_type relationships.
 *
 * @param unknown_type $form_id
 * @param unknown_type $form_state
 */
function calais_admin_settings_submit($form, &$form_state) {

  system_settings_form_submit($form, $form_state);

  // Also, set vocabulary-node relationships
  $node_types = node_get_types('types', NULL, TRUE);
  $all_vocabularies = calais_get_entity_vocabularies();
  foreach ($node_types as $nt) {
    $key = drupal_strtolower($nt->type);
    $state = variable_get('calais_node_'. $key .'_process', CALAIS_PROCESS_NO);
    foreach ($all_vocabularies as $entity => $vid) {
      // Clean-up
      db_query("DELETE FROM {vocabulary_node_types} WHERE vid='%d' and type='%s'", $vid, $key);
    }
    if ($state !== CALAIS_PROCESS_NO) {
      // assign all configured calais vocabs to this node type
      $node_vocabularies = calais_get_entity_vocabularies($key);
      if(!empty($node_vocabularies)) {
        foreach ($node_vocabularies as $entity => $vid) {
          db_query("INSERT INTO {vocabulary_node_types} (vid, type) values('%d','%s') ", $vid, $key);
        }
      }
    }
  }
}

/**
 * Build the administration page for submitting all nodes of a specific Content Types to Calais.
 */
function calais_bulk_process() {

  $form = array();

  $node_types = node_get_types('types', NULL, TRUE);
  $node_types = array();
  foreach (node_get_types('types', NULL, TRUE) as $type) {
    $node_types[$type->type] = $type->name;
  }

  $form["calais_bulk_type"] = array(
    '#type' => 'select',
    '#title' => t('Content Type'),
    '#options' => $node_types,
    '#description' => t('Choose a content type whose Nodes will be bulk processed by Calais.'),
  );
  
  $form["calais_bulk_limit"] = array(
    '#type' => 'select',
    '#title' => t('Nodes per batch'),
    '#options' => drupal_map_assoc(array('1', '5', '10', '25', '50', '100')),
    '#description' => t("Select the number of nodes per batch (needed for larger jobs where a 'single shot' isn't feasible)."),
  );

  $form["calais_bulk_threshold"] = array(
    '#type' => 'textfield',
    '#title' => t('Term Relevance Threshold for Calais'),
    '#size' => 5,
    '#default_value' => 0.650,
    '#description' => t('Determine how relevant a term must be in order for Calais to suggest it for a particular node.  Based on a 0.00-1.00 scale, with 0.00 being least relevant (i.e. many terms appear).'),
  );

  $form["calais_bulk_throttle"] = array(
    '#type' => 'checkbox',
    '#title' => t('Throttle processing'),
    '#default_value' => TRUE,
    '#description' => t('Limit the number of requests per second sent to the Calais web service, in order to ensure that all nodes are processed by Calais (recommended).'),
  );

  $form['submit'] = array('#type' => 'submit', '#value' => t('Bulk Process'));
  $form['#submit'][] = 'calais_bulk_process_submit';


  // Generate a summary of total node counts per content type
  $header = array(
    t('Type'),
    t('Node Count'),
    t('Status'),
  );
  $rows = array();
  $result = db_query("SELECT type, COUNT(*) as count FROM {node} GROUP BY type");  
  $progress = variable_get("calais_bulk_progress", array());
  while ($row = db_fetch_object($result)) {
    if(isset($node_types[$row->type])) {
      $rows[] = array(
          $node_types[$row->type],
          $row->count,
          isset($progress[$row->type]) 
            ? t('Interrupted, will continue (!clear)', array('!clear' => l('Start Over', "admin/settings/calais/bulk-process/clear/{$row->type}", array('query' => drupal_get_destination())))) 
            : t('Ready'),
      );
    }
  }
  $form['summary'] = array(
    '#type' => 'markup',
    '#value' => theme('table', $header, $rows),
  );

  return $form;
}

function calais_bulk_process_validate($form, &$form_state) {
  $threshold = round($form_state['values']['calais_bulk_threshold'] * 1000);
  if ($threshold < 0 || $threshold > 1000) {
    form_set_error('calais_bulk_threshold', t('Threshold must be between 0.000 and 1.000'));
  }
}

/**
 * Submit handler for the Bulk Processing Form.
 */
function calais_bulk_process_submit($form, &$form_state) {

  $values = $form_state['values'];
  $type = $values['calais_bulk_type'];
  $limit = $values['calais_bulk_limit'];
  $throttle = $values['calais_bulk_throttle'];
  $threshold = $values['calais_bulk_threshold'];
 
  $batch = array(
    'operations' => array(
      array('calais_batch_process', array($type, $limit, $throttle, $threshold)),
    ),
    'finished' => 'calais_batch_finished',
    'title' => t('Calais Bulk Processing'),
    'init_message' => t('Bulk processing of nodes is starting.'),
    'error_message' => t('Calais Bulk Processing has encountered an error.'),
  );
  batch_set($batch);
}

/**
 * Clear a particular content types progress indicator for Calais bulk processing
 * 
 * @param $type
 *    The machine readable node type
 */
function calais_bulk_clear($type) {
  $progress = variable_get("calais_bulk_progress", array());
  unset($progress[$type]);
  variable_set("calais_bulk_progress", $progress);
  watchdog('calais', "Cleared bulk progress indicator for %type nodes", array('%type' => $type));
  drupal_goto();
}

/**
 * Process nodes in bulk by submitting blocks to Calais for metadata association.
 *  
 * @param type
 *   The node type that we want to process
 *
 * @param limit
 *   The number of nodes to process in each pass.
 *
 * @param throttle
 *   Flag indicating if Calais requests should be throttled (to stay below quota limits).
 *
 * @param threshold
 *   Relevence threshold for Calais terms.
 *
 * @param context
 *   $context is an array that will contain information about the
 *   status of the batch. The values in $context will retain their
 *   values as the batch progresses.
 *
 * @param $context['sandbox']
 *   Use the $context['sandbox'] rather than $_SESSION to store the
 *   information needed to track information between successive calls.
 *   The values in the sandbox will be stored and updated in the database
 *   between http requests until the batch finishes processing. This will
 *   avoid problems if the user navigates away from the page before the
 *   batch finishes.
 *
 * @param $context['results']
 *   The array of results gathered so far by the batch processing.
 *   The current operation can append its own.
 *
 * @param $context['message']
 *   A text message displayed in the progress page.
 *
 * @param $context['finished']
 *   A float number between 0 and 1 informing the processing engine
 *   of the completion level for the operation.
 *   1 (or no value explicitly set) means the operation is finished
 *   and the batch processing can continue to the next operation.
 */
function calais_batch_process($type, $limit, $throttle, $threshold, &$context) {
  // Get any in progress bulks
  $progress = variable_get("calais_bulk_progress", array());
  $lastnid = $progress[$type];

  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    if(isset($lastnid)) {
      $context['sandbox']['current_node'] = $lastnid;
      $context['sandbox']['max'] = db_result(db_query("SELECT COUNT(DISTINCT nid) FROM {node} WHERE type = '%s' and nid > %d", $type, $lastnid));
    }
    else {
      $context['sandbox']['current_node'] = 0;
      $context['sandbox']['max'] = db_result(db_query("SELECT COUNT(DISTINCT nid) FROM {node} WHERE type = '%s'", $type));
    }
  }
  
  // With each pass through the callback, retrieve the next group of nids.
  $current = $context['sandbox']['current_node'];
  $result = db_query_range("SELECT nid FROM {node} WHERE type = '%s' AND nid > %d ORDER BY nid ASC", $type, $current, 0, $limit);
  while ($row = db_fetch_array($result)) {

    $node = node_load($row['nid'], NULL, TRUE);
    calais_process_node($node, CALAIS_PROCESS_AUTO, $threshold);
    $progress[$type] = $node->nid;
    variable_set("calais_bulk_progress", $progress);

    if($throttle) {
      usleep(100000);
    }

    $context['results'][] = $node->title;
    $context['sandbox']['progress']++;
    $context['sandbox']['current_node'] = $node->nid;
    $context['message'] = t('Now processing %node', array('%node' => $node->title));    
  }

  watchdog('calais', "Bulk processed {$limit} %type nodes", array('%type' => $type));

  // Inform the batch engine that we are not finished,
  // and provide an estimation of the completion level we reached.
  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
  else {
    // We are finished so remove our progress tracker
    unset($progress[$type]);
    variable_set("calais_bulk_progress", $progress);
  }
}

/**
 * Batch 'finished' callback
 */
function calais_batch_finished($success, $results, $operations) {
  if ($success) {
    $message = count($results) .' nodes processed.';
    $message .= theme('item_list', $results);
  }
  else {
    $error_operation = reset($operations);
    $message = 'An error occurred while processing '. $error_operation[0] .' with arguments :'. print_r($error_operation[0], TRUE);
  }
  drupal_set_message($message);
}