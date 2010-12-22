<?php
/**
 * Ctools Plugin definition for the export_ui
 */
$plugin = array(
  'schema' => 'opencalais_preset',
  'access' => 'administer opencalais',
  'menu' => array(
    'menu prefix' => 'admin/config/content/opencalais',
    'menu item' => 'presets', 
    'menu title' => 'Open Calais Preset', 
    'menu description' => 'Administer OpenCalais',
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
