<?php
/**
 * @file theme.inc
 */

/**
 *
 */ 
function theme_opencalais_preset_form($vars) {
  $buttons = drupal_render($vars['form']['buttons']);
  $header = array(
    t('Category'),
    t('Enabled?'),
    t('Threshold'),
  );
  $rows = array();
  foreach (element_children($vars['form']['info']['config']['entities']) as $entity) {
  
    $settings = $vars['form']['info']['config']['entities'][$entity];
    unset($settings['enabled']['#title']);
    unset($settings['threshold']['#title']);
    $rows[] = array(
      opencalais_api_make_readable($entity),
      drupal_render($settings['enabled']),
      drupal_render($settings['threshold']),
    );
  }
  $entity_table = theme('table', array('header' => $header, 'rows' => $rows));
  unset($vars['form']['info']['config']['entities']);
  
  $output = drupal_render_children($vars['form']);
  $output .= $entity_table;
  $output .= $buttons;
  return $output;  
}
