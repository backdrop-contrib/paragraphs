<?php

/**
 * @file
 */

/**
 * Outputs information about a Layout Paragraphs template . Used in the
 * Paragraphs layout template listing page.
 *
 * @param $variables
 *   An array with a single value: "template_info", which contains information
 *   about the layout template.
 *
 * @return string
 */
function theme_layout_paragraphs_template_info($variables) {
  $template_info = $variables['template_info'];

  // Get the label and machine name.
  $label = theme('label_machine_name__layout_template', array(
    'label' => $template_info['title'],
    'machine_name' => $template_info['name'],
  ));

  $output = '<div class="layout-info">';
  $output .= $template_info['preview'];
  $output .= '<div class="layout-detail">' . $label . '</div>';
  $output .= '</div>';

  return $output;
}

/**
 * Outputs an individual option label for selecting a layout paragraph template.
 */
function theme_layout_paragraphs_template_option($variables) {
  $template_info = $variables['template_info'];

  $output = '';
  $output .= '<div class="layout-template">';
  $output .= $template_info->preview;
  // $output .= '<div class="layout-caption">' . check_plain($template_info->title) . '</div>';
  $output .= '</div>';
  return $output;
}

/**
 * Outputs items from a Paragraphs field in Layout form.
 *
 * We:
 *  - Look up the layout information in layout_paragraphs, including the ID.
 *  - Look up the block placements from layout_paragraphs_blocks.
 *  - Compare blocks to the items sent in the $variables array; if any are
 *    missing, we add them to the default region.
 *  - Render the layout using the layout_paragraphs renderer.
 */
function theme_layout_paragraphs_items($variables) {
  module_load_include('inc', 'layout_paragraphs', 'layout_paragraphs.admin');
  $layout = layout_paragraphs_layout_load($variables['layout_name'], TRUE, 'database');
  $renderer = layout_create_renderer('paragraphs_standard', $layout);
  // @todo: Roughly render them for now, until we can polish.
  // @todo: It's possible individual Paragraphs are already rendered and we
  // could avoid re-rendering them -- follow the theme processing to be sure.
  $output = '';
  $output .= '<div class="layout-paragraphs--wrapper">';
  $output .= $renderer->render();
  $output .= '</div>';

  return $output;
}
