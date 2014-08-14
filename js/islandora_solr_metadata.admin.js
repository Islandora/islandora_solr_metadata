/**
 * @file
 * Script for Islandora Solr Metadata admin.
 *
 * Heavily crimped from the JS in the Islandora Solr Search module.
 */
(function ($) {
    Drupal.behaviors.islandoraSolrMetadataConfigure = {
        attach: function(context, settings) {
            $(".islandora-solr-metadata-configure-link").each(function () {
                if (!$(this).hasClass('processed')) {
                    $('.islandora-solr-metadata-configure-link').click(function() {
                        $(this).parent('.islandora-solr-metadata-operations').next('td').find('input').trigger('click');
                        return false;
                    });
                    $(".islandora-solr-metadata-configure-link").addClass('processed');
                }
            });
        }
    };

    // function for the  dialog box
    Drupal.behaviors.islandoraSolrMetadataDialog = {
        attach: function(context, settings) {
            // Create a jQuery UI dialog, but leave it closed.
            var dialog_area = $('#islandora-solr-metadata-admin-dialog', context);
            dialog_area.dialog({
                'autoOpen': false,
                'dialogClass': 'islandora-solr-metadata-admin-dialog',
                'modal': true,
                'position': 'center',
                'resizable': false,
                'width': 900,
                'height': 500,
                'draggable': false
            });
        }
    };

    // resize dialog box
    // @TODO: some small bugs in here.
    Drupal.islandoraSolrMetadata = {};
    Drupal.islandoraSolrMetadata.resizeModal = function() {
        // calculate dimensions
        // dom elements
        var $modal = $('#islandora-solr-metadata-admin-dialog');
        var $scroll = $('#islandora-solr-metadata-admin-dialog-form');
        // window
        var windowWidth = $(window).width();
        var windowHeight = $(window).height();
        // max modal values
        var maxWidth = parseInt(windowWidth * .6); // 60%
        var maxHeight = parseInt(windowHeight * .8); // 80%
        // get scroll height
        var scrollHeight = $scroll.height();
        // get modal height
        var modalHeight = $modal.height();
        // get modal height
        var modalHeightCalc = scrollHeight + 98; // scroll + header & footer

        // if maximum height is smaller than the calculated height
        if (maxHeight < modalHeightCalc) {
            // @TODO, when fieldset is larger than $scroll, this gets set correctly and stays that way. When fieldset isn't quite that big and doesn't get this assigned, collapsing and expanding again, will cause the dialog to jump, because no fixed height is set.
            if (scrollHeight > maxHeight - 98) {
                scrollHeight = maxHeight - 98;
            }

            $scroll.css({
                'height': scrollHeight + 'px', // 1) 494 2) 465
                'max-height': scrollHeight + 'px'
            });
        }

        // set dialogHeight
        var dialogHeight = modalHeightCalc;
        // make sure the dialogHeight is never more than the maximum height
        if (dialogHeight > maxHeight) {
            dialogHeight = modalHeightCalc;
        }

        // apply dimensions
        $modal.dialog('option', 'width', maxWidth);
        $modal.dialog('option', 'height', dialogHeight);
        $modal.dialog('option', 'position', 'center');

    }

    // function for the dialog box window resize event
    Drupal.behaviors.islandoraSolrMetadataDialogResize = {
        attach: function(context, settings) {
            // apply dialog dimensions on load
            Drupal.islandoraSolrMetadata.resizeModal();
            // apply dialog dimensions on resize
            $(window).resize(function() {
                Drupal.islandoraSolrMetadata.resizeModal();
            });

            // on fieldset collapse event
            // snippet from views ui, which has taken part of a snippet from collapse.js. Yo dawg..
            if (!this.collapseReplaced && Drupal.collapseScrollIntoView) {
                this.collapseReplaced = true;
                Drupal.collapseScrollIntoView = function (node) {
                    for (var $parent = $(node); $parent.get(0) != document && $parent.size() != 0; $parent = $parent.parent()) {
                        if ($parent.css('overflow') == 'scroll' || $parent.css('overflow') == 'auto') {

                            // If the modal is already at the max height, don't bother with
                            // this since the only reason to do it is to grow the modal.
                            if ($('.views-ui-dialog').height() < parseInt($(window).height() * .8)) {
                                Drupal.islandoraSolrMetadata.resizeModal();
                            }
                            return;
                        }
                    }
                    var h = document.documentElement.clientHeight || document.body.clientHeight || 0;
                    var offset = document.documentElement.scrollTop || document.body.scrollTop || 0;
                    var posY = $(node).offset().top;
                    var fudge = 55;
                    if (posY + node.offsetHeight + fudge > h + offset) {
                        if (node.offsetHeight > h) {
                            window.scrollTo(0, posY);
                        }
                        else {
                            window.scrollTo(0, posY + node.offsetHeight - h + fudge);
                        }
                    }
                };
            }


        }
    };

    // Add dialog form values to the drupal ajax settings
    // @see http://drupal.org/node/1028410#comment-4301262
    // @TODO: check if this might be a better approach: http://drupal.stackexchange.com/questions/9920/how-to-extend-or-hook-drupal-form-ajax/10191#10191
    $.fn.islandoraSolrMetadataDialogValues = function(data) {
        // dialog ajax id
        var dialogAjaxId = data.id;
        // add values as JSON, so we can pass multi dimensional arrays
        Drupal.ajax[dialogAjaxId].options.data._dialog_values = JSON.stringify(data.values);
    };

    // attach behaviors to make sure all javascript is added to the form in the dialog
    $.fn.islandoraSolrMetadataAttachBehaviors = function() {
        Drupal.attachBehaviors();
    };

    // resize modal dialog
    $.fn.islandoraSolrMetadataResizeModal = function() {
        Drupal.islandoraSolrMetadata.resizeModal();
    };

})(jQuery);
