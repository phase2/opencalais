<?php
// $Id: Calais.inc,v 1.1.2.16.2.2 2009/12/14 21:50:26 febbraro Exp $
/**
 * @file Calais.inc
 * The main interface to the calais web service
 */

interface Calais {
  
  /**
   * Analyze the content via Calais.
   *
   * @param $content 
   *    The content to ship off to Calais for analysis
   * @return 
   *    The processed Calais results. The raw RDF result is contained in the $this->rdf field.
   */
  public function analyze($content);
  
  /**
   * Analyze the provided content, passing it to Calais in XML format for more accurate data processing.
   *
   * @param $title  
   *    The title of the content to process
   * @param $body   
   *    The body ofd the content to process
   * @param $date   
   *    The date of the content, if left blank/null analysis will use "today"
   * @return 
   *    The processed Calais results. The raw RDF result is contained in the $this->rdf field.
   */
  public function analyzeXML($title, $body, $date);

  /**
   * Analyze the provided content, passing it to Calais in HTML format .
   *
   * @param $content  
   *    The HTML content to process
   * @return 
   *    The processed Calais results. The raw RDF result is contained in the $this->rdf field.
   */
  public function analyzeHTML($content);
}
