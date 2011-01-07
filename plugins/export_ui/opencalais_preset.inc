<?php
/**
 * Ctools Plugin definition for the export_ui
 */
$plugin = array(
  'schema' => 'opencalais_preset',
  'access' => 'administer opencalais',
  
  // Will default to admin/structure/[menu item]
  'menu' => array(
    'menu item' => 'opencalais-presets', 
    'menu title' => 'OpenCalais Presets', 
    'menu description' => 'Administer OpenCalais Configuration for Entities',
  ),

  'title singular' => t('opencalais preset'),
  'title singular proper' => t('OpenCalais Preset'),
  'title plural' => t('opencalais presets'),
  'title plural proper' => t('OpenCalais Presets'),

  'form' => array(
    'settings' => 'opencalais_preset_form',
    'submit'   => 'opencalais_preset_form_submit',
  ),

);
