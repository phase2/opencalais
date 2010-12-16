<?php
// $Id: calais_handler_argument_calais_term_vid.inc,v 1.1.2.1 2009/02/09 20:23:37 febbraro Exp $

/**
 * Argument handler to accept a vocabulary id.
 */
class calais_handler_argument_calais_term_vid extends views_handler_argument_numeric {

  /**
   * Return all entity type argument names.
   */
  function title_query() {
    $titles = array();
    $placeholders = db_placeholders($this->value);

    $result = db_query("SELECT name FROM {vocabulary} WHERE vid IN ($placeholders)", $this->value);
    while ($vocab = db_fetch_object($result)) {
      $titles[] = check_plain($vocab->name);
    }
    return $titles;
  }
}