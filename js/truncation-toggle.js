(function ($) {
  "use strict";

  Drupal.behaviors.islandoraSolrMetadataTruncationToggle = {
    attach: function (context, settings) {
      $('.toggle-wrapper', context).once('truncation-toggle').css({
        'list-style-type': 'none',
        'padding': 0,
        'margin': 0
      });
      $('.toggle-wrapper .toggler', context).once('truncation-toggle', function(){
        var $this = $(this);
        $this.click(function (){
          $this.closest('.toggle-wrapper').find('> li').toggle();
        });
      });
      $('.toggle-wrapper > li', context).once('truncation-toggle').hide().first().show();
    }
  };
})(jQuery);
