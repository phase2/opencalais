/**
 * @file opencalais.admin.js
 *    This file contains all javascript associated with the OpenCalais preset admin screens.
 */
(function ($) {

Drupal.behaviors.opencalaisEntityConfig = {
  attach: function (context) {
    $(".threshold_slider", context).each(function() {
      
      $(this).hide();
 
      var threshold = $(this);
      var container = $(this).parent("div");     
      var name = $(this).attr('name');
      var slider_name = name + '-slider';
      var slider = $('<div>').attr('id', slider_name).insertBefore($(this));
      var label_id = slider_name + '-label';
      var label = $('<div>').html(threshold.val()).attr({id: label_id, class: 'slider_label'}).insertAfter(slider);      
      slider.slider({
        min: 0.00,
  			max: 1.00,
  			step: 0.01,
  			value: threshold.val(),
        slide: function(event, ui ) {
          threshold.val(ui.value);
          label.html(ui.value);
        },
      });
    });
  }
};

})(jQuery);