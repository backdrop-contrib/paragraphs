/**
 * @file
 * Provides JavaScript for Paragraphs.
 */

(function ($) {

  /**
   * Allows submit buttons in entity forms to trigger uploads by undoing
   * work done by Backdrop.behaviors.fileButtons.
   */
  Backdrop.behaviors.paragraphs = {
    attach: function (context) {
      if (Backdrop.file) {
        $('input.paragraphs-add-more-submit', context).unbind('mousedown', Backdrop.file.disableFields);
      }
    },
    detach: function (context) {
      if (Backdrop.file) {
        $('input.form-submit', context).bind('mousedown', Backdrop.file.disableFields);
      }
    }
  };

  /**
   * Close any open "more actions" menus.
   */
  Backdrop.behaviors.closeMoreActions = {
    attach: function (context) {
      $('body', context).on('click', Backdrop.paragraphs.closeActionMenu);
    },
    detach: function (context) {
      $('body', context).on('click', Backdrop.paragraphs.closeActionMenu);
    }
  };

  Backdrop.paragraphs = Backdrop.paragraphs || {

    /**
     * Close any open actions menus if the click is outside of the menu.
     */
    closeActionMenu: function (event) {
      var $moreactions = $('.paragraphs-actions-more');
      var $body = $('body');
      var insideMoreActions = $.contains(event.target, $body);
      if (!insideMoreActions) {
        $moreactions.removeAttr('open');
      }
    }

  };

})(jQuery);
