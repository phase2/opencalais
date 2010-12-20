<?php
/**
 * @file opencalais.admin.inc
 */
 
/**
 * Present some configuration options.
 */ 
function opencalais_admin_general_settings() {
  $form = array();
  $form['opencalais_tags_in_verticaltab']  = array(
    '#type' => 'checkbox', 
    '#title' => t('Collect Tags into Vertical Tab'), 
    '#description' => t('This will move all of the OpenCalais related Tag fields on the edit form into a Vertical Tab toward the bottom of the form entitled "Show OpenCalais Tags"'), 
    '#default_value' => variable_get('opencalais_tags_in_verticaltab'),
  );    
  $form['opencalais_lookup_uri'] = array(
    '#type' => 'checkbox',
    '#title' => t('Lookup Linked Open Data URI'),
    '#description' => t('If checked, an additional HTTP request will be made for each disambiguated entity to find the Linked Open Data URIs to DBPedia, Freebase, etc.  It does involve additional requests to external servers.  If you don\'t know what this is, you should leave it disabled.'),
    '#default_value' => variable_get('opencalais_lookup_uri'),
  );
  return system_settings_form($form);
}