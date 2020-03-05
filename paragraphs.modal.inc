<?php

/**
 * @file
 * Provides callbacks for modal edits.
 */

/**
 * Page callback to allow editing a single paragraphs item in a modal.
 *
 */

function paragraphs_edit_modal($paragraph_item) {
  return 'This form should allow editing of Paragraph item ' . check_plain($paragraph_item);
}