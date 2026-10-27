<?php

/**
 * @file
 * Page callbacks for Paragraphs i18n integration module.
 */

/**
 * Translate page callback.
 *
 * @param stdClass $paragraphs_bundle
 *  The paragraphs type object.
 * @param stdClass $language
 *  The language object.
 *
 * @return array
 *  The form array.
 */
function paragraphs_i18n_page_translate($paragraphs_bundle, $language = NULL) {
  module_load_include('inc', 'i18n_string', 'i18n_string.pages');
  return i18n_string_translate_page_object('paragraphs_bundle', $paragraphs_bundle, $language);
}
