(function($, Backdrop) {
    Backdrop.ajax.prototype.commands.paragraphs_separate_edit_message_hide = function(ajax, response, status) {
        $('.paragraphs-separate-edit-message').delay(1000).fadeOut();
        //alert(response.value1);
    }
})(jQuery, Backdrop);