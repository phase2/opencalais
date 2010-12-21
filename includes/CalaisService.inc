<?php
// $Id: Calais.inc,v 1.1.2.16.2.2 2009/12/14 21:50:26 febbraro Exp $
/**
 * @file Calais.inc
 * The main interface to the calais web service
 */

class CalaisService implements Calais {
  
  const PATH = '/enlighten/rest/';
  
  private $defaults = array(
    'protocol' => 'http',
    'contentType' => 'TEXT/HTML',
    'outputFormat' => 'XML/RDF',
    'externalID' => '',
    'submitter' => 'Drupal',
    'calculateRelevanceScore' => 'true',
    'enableMetadataType' => 'SocialTags',
    'allowSearch' => 'false',
    'allowDistribution' => 'false',
    'caller' => 'Drupal',
  );
  
  public $parameters;
  public $rdf;
  public $triples = array();
  public $flatTriples = array();
  public $keywords = array();
  
  /**
   * Constructs an instance of the Calais facade.
   *
   * Valid parameters are specified in the options array as key/value pairs with the
   * parameter name being the key and the parameter setting being the value
   * e.g. array('allowSearch' => 'false')
   *
   * @param options  An array of parameter options for the Calais Web Service. 
   *                  These will override the defaults.
   *
   * @see http://opencalais.com/APIcalls#inputparameters
   */
  function __construct($options = array()) {
    $this->defaults['externalID'] = time();
    $this->defaults['host'] = variable_get('opencalais_api_server', 'api1.opencalais.com');
    
    $this->parameters = array_merge($this->defaults, $options);
  }
  
  /**
   * Analyze the provided content, passing it to Calais in XML format for more accurate data processing.
   *
   * @param $title  The title of the content to process
   * @param $body   The body ofd the content to process
   * @param $date   The date of the content, if left blank/null analysis will use "today"
   * @return The processed Calais results. The raw RDF result is contained in the $this->rdf field.
   */
  public function analyzeXML($title, $body, $date) {
    $content = $this->build_xml_content($title, $body, $date);
    $this->parameters['contentType'] = 'TEXT/XML';
    return $this->analyze($content);
  }
  
  /**
   * Analyze the provided content, passing it to Calais in HTML format .
   *
   * @param $content  
   *    The HTML content to process
   * @return 
   *    The processed Calais results. The raw RDF result is contained in the $this->rdf field.
   */
  public function analyzeHTML($content) {
    $this->parameters['contentType'] = 'TEXT/HTML';
    return $this->analyze($content);
  }

  /**
   * Analyze the content via Calais.
   *
   * @param $content The content to ship off to Calais for analysis
   * @return The processed Calais results. The raw RDF result is contained in the $this->rdf field.
   */
  public function analyze($content) {

    $headers = array('Content-Type' => 'application/x-www-form-urlencoded');
    $data    = array(
      'licenseID' => variable_get('opencalais_api_key', NULL),
      'content' => $content,
      'paramsXML' => $this->build_xml_params(),
    );
    $data_enc = http_build_query($data, '', '&');
    $uri = $this->parameters['protocol'] . '://' . $this->parameters['host'] . self::PATH;
    $req = array(
      'headers' => $headers,
      'method' => 'POST',
      'data' => $data_enc,
    );
    $ret = drupal_http_request($uri, $req);
    if (isset($ret->error)) {
      self::log_calais_error($ret);
      return array();
    }

    $this->rdf = $ret->data;
    $this->processor = new CalaisRdfProcessor();
    $this->keywords = $this->processor->parse_rdf($this->rdf);    
    $this->triples = $this->processor->triples;
    if (isset($this->processor->flatTriples)) {  
      $this->flatTriples = $this->processor->flatTriples;
    }
    return $this->keywords;    
  }

  /**
   * Build the XML Parameters required by the Calais Web-Service
   *
   * @return XML document of Calais parameters.
   */
  protected function build_xml_params() {
    $attrs = (object)$this->parameters;
    $ret = <<<EOD
<c:params xmlns:c="http://s.opencalais.com/1/pred/" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <c:processingDirectives c:contentType="{$attrs->contentType}" 
                          c:outputFormat="{$attrs->outputFormat}"
                          c:enableMetadataType="{$attrs->enableMetadataType}"
                          c:calculateRelevanceScore="{$attrs->calculateRelevanceScore}"
  >
  </c:processingDirectives>
  <c:userDirectives c:allowDistribution="{$attrs->allowDistribution}" 
                    c:allowSearch="{$attrs->allowSearch}"   
                    c:externalID="{$attrs->externalID}"
                    c:submitter="{$attrs->submitter}"
  >
  </c:userDirectives>
  <c:externalMetadata>
    <rdf:description>
      <c:caller>{$attrs->caller}</c:caller>
    </rdf:description>
  </c:externalMetadata>
</c:params>
EOD;
    return $ret;
  }
  
  /**
   * Build the XML document request format expected by Calais
   *
   * @return an xml string to be submitted to Calais
   * @see http://opencalais.com/APIcalls#inputcontentformat
   */
  protected function build_xml_content($title, $body, $date) {
    $req = "<DOCUMENT>";
    $req .= "<TITLE><![CDATA[$title]]></TITLE>";
    $req .= "<DATE>$date</DATE>";
    $req .= "<BODY><![CDATA[$body]]></BODY>";
    $req .= "</DOCUMENT>";
    return $req;
  }  
  
  private static function log_calais_error($ret) {
    $msg = t('OpenCalais processing error: @msg', array('@msg' => $ret->data));
    drupal_set_message($msg, 'error');
    watchdog('opencalais', 'OpenCalais processing error: (@code - @error) @msg', array('@code' => $ret->code, '@error' => $ret->error, '@msg' => $ret->data), WATCHDOG_ERROR);
  }
}
