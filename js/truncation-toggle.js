(function ($) {
  "use strict";

  Drupal.behaviors.islandoraSolrMetadataTruncationToggle = {
    attach: function (context, settings) {
      $('.toggle-wrapper', context).once('truncation-toggle', function() {
        var $this = $(this);
        $this.css({
          'list-style-type': 'none',
          'padding': 0,
          'margin': 0
        }).find('> li').hide().first().show();
      });
      $('.toggle-wrapper .toggler', context).once('truncation-toggle', function(){
        var $this = $(this);
        $this.click(function (){
          $this.closest('.toggle-wrapper').find('> li').toggle();
        });
      });
    }
  };
})(jQuery);
