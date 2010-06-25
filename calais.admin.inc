<?php
/*
  Copyright (C) 2008-2010 by Phase2 Technology.

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
function calais_basic_admin_settings() {
  $form = array();



  return $form;
}


/**
 * Build the administration page for Calais and Content Types
 */
function calais_advanced_admin_settings() {
  $form = array();



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
