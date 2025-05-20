<?php

/**
 * @file
 * Holds functions for the Layout Paragraphs widgets.
 */

/**
 * Implements hook_field_widget_info().
 */
function layout_paragraphs_field_widget_info() {
  return array(
    'layout_paragraphs' => array(
      'label' => t('Layout UI'),
      'field types' => array('paragraphs'),
      'behaviors' => array(
        'multiple values' => FIELD_BEHAVIOR_CUSTOM,
        'default value' => FIELD_BEHAVIOR_NONE,
      ),
    ),
  );
}

/**
 * Implements hook_field_widget_settings_form().
 */
function layout_paragraphs_field_widget_settings_form($field, $instance) {
  $widget = $instance['widget'];
  $settings = $widget['settings'];
  $form = array();

  if ($widget['type'] == 'layout_paragraphs') {
    $options = array(
      'default' => t('Default'),
    );
    $templates = layout_flexible_template_load_all('paragraphs');
    foreach ($templates as $template) {
      $options[$template->name] = $template->title;
    }
    $form['available_layout_templates'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Allowed layout templates'),
      '#description' => t('Which layout template(s) should be available for this Paragraphs field?'),
      '#description_display' => 'after',
      '#options' => $options,
      '#default_value' => (!empty($settings['available_layout_templates'])) ? $settings['available_layout_templates'] : 'default',
    );
  }
  return $form;
}

/**
 * Implements hook_field_widget_form().
 */
function layout_paragraphs_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
  switch ($instance['widget']['type']) {
    case 'layout_paragraphs':
      // Return paragraphs_field_multiple_value_form($field, $instance, $langcode, $items, $form, $form_state, $delta, $element);
      // Add the layout functionality directly to the form; return the placeholder for the Paragraphs data.
      return layout_paragraphs_edit_content($form, $form_state, $field, $instance, $langcode, $items, $delta, $element);
  }
}

 /**
 * Form constructor for the Paragraphs layout configuration form.
 *
 * @param array $form
 *   An associative array containing the structure of a portion of the form.
 * @param array $form_state
 *   A keyed array containing the current state of the form.
 * @param array $field
 *   The field structure.
 * @param array $instance
 *   The field instance.
 * @param string $langcode
 *   The language associated with $items.
 * @param array $items
 *   Array of default values for this field.
 * @param int $delta
 *   The order of this item in the array of subelements (0, 1, 2, etc).
 * @param array $element
 *   A form element array containing basic properties for the widget.
 *
 * @see layout_paragraphs_menu()
 * @see layout_paragraphs_edit_content_validate()
 * @see layout_paragraphs_edit_content_submit()
 * @ingroup forms
 */
function layout_paragraphs_edit_content(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
  if ($instance['widget']['type'] == 'layout_paragraphs') {
    $return = [];
    form_load_include($form_state, 'inc', 'layout_paragraphs', 'layout_paragraphs.admin');
    form_load_include($form_state, 'inc', 'layout_paragraphs', 'layout_paragraphs.context.admin');
    form_load_include($form_state, 'inc', 'layout', 'layout.admin');

    $parents = $form['#parents'];
    $field_name = $field['field_name'];
    $langcode = (!empty($langcode)) ? $langcode : LANGUAGE_NONE;

    $form['#attached']['js'][] = backdrop_get_path('module', 'layout') . '/js/layout.admin.js';
    // $form['#attached']['css'][] = backdrop_get_path('module', 'mini_layouts') . '/css/mini-layouts.admin.css';
    $form['layout_paragraphs'] = array(
      '#type' => 'container',
      '#attributes' => array(
        'id' => array('ajax-layout-paragraphs-templates'),
      ),
    );
    $all_templates = layout_flexible_template_load_all('paragraphs');
    $available_templates = $instance['widget']['settings']['available_layout_templates'];
    $entity_type = $element['#entity_type'];
    $entity_id = $element['#entity']->id() ?? 0;
    $form['layout_paragraphs']['#field'] = $field;
    $form['layout_paragraphs']['#instance'] = $instance;
    $form['layout_paragraphs']['#entity'] = $element['#entity'];
    if (empty($form_state['layout_paragraphs']['#entity_tempstore_id'])) {
      $form_state['layout_paragraphs']['#entity_tempstore_id'] = (!empty($entity_id)) ? $entity_id : (new Uuid())->generate();
    }
    if (!empty($form_state['layout_paragraphs']['select_template']['layout_paragraphs_templates'])) {
      $default_template = $form_state['layout_paragraphs']['select_template']['layout_paragraphs_templates'];
    }
    else {
      $default_template = _layout_paragraphs_determine_layout_template($entity_id, $instance['entity_type'], $instance['bundle'], $field_name, $available_templates);
    }
    $active_templates = 0;
    foreach ($available_templates as $template_name => $template_status) {
      if ($template_status != 0) {
        $active_templates++;
        break;
      }
    }
    $layout_name = _layout_paragraphs_get_layout_name($instance, $form_state['layout_paragraphs']['#entity_tempstore_id']);
    $layout = layout_paragraphs_instance_tempstore_load($layout_name);
    if (empty($entity_id) || !$layout) {
      $layout = layout_paragraphs_layout_load($layout_name);
      $layout->locked = FALSE;
    }

    $available_templates = ($active_templates) ? $available_templates : array_keys($all_templates);
    $options = [];
    if (!empty($available_templates)) {
      $template_options = [];
      // ($instance['widget']['settings']['available_layout_templates']);
      foreach ($available_templates as $template_name) {
        if (!empty($all_templates[$template_name])) {
          $options[$template_name] = $all_templates[$template_name]->title;
          $template_options[$template_name] = $all_templates[$template_name];
        }
      }
      if (count($options) > 1) {
        module_load_include('inc', 'layout_paragraphs', 'layout_paragraphs.admin');
        $form['layout_paragraphs']['select_template'] = layout_paragraphs_select_template($field, $instance, $template_options, $default_template);
      }
      elseif (count($options) == 1) {
        // Only one choice possible, so we will use it automatically.
        $template_name = key($options);
        $form['layout_paragraphs']['select_template']['layout_paragraphs_templates'] = array(
          '#type' => 'hidden',
          '#title' => t('Layout template'),
          '#description' => t('Which layout template should be used?'),
          '#value' => (!empty($template_name)) ? $template_name : $default_template,
          '#required' => TRUE,
        );
        $layout->layout_template = $template_name;
      }
    }
    else {
      // @todo: It should be impossible to have no templates available if we've
      // prevented default from being removed or disabled. Verify, then remove
      // this message.
      $form['layout_paragraphs']['tempmarkup'] = array(
        '#markup' => '<div class="messages warning"><p>' . t('No layout templates are available for this field. Using default.') . '</p></div>',
      );
    }

    $layout_form = $layout_form_state = [];
    if (!$form_state['submitted']) {
      layout_paragraphs_set_layout_tempstore($layout);
    }
    $layout_content = layout_paragraphs_content_form($layout_form, $layout_form_state, $layout, 'paragraphs_editor');
    $form['layout_paragraphs']['layout'] = $layout_content;
    $form['layout_paragraphs']['element'] = $element;

    if ($entity_type == 'taxonomy_term') {
      // @todo: Figure out why this seems to be needed for terms.
      // Maybe because the submit handler is not explicitly set in taxonomy_form_term?
      // Paragraphs don't seem to be saving on new terms, unless you save the term,
      // then edit it again. Maybe tempstore needs to be set earlier?
      $form['#submit'] = [];
      $form['#submit'][] = 'taxonomy_form_term_submit';
    }
    $form['#submit'][] = 'layout_paragraphs_edit_content_submit';

    layout_paragraphs_set_layout_tempstore($layout);
    return $return;
  }
}

/**
 * Submit handler for layout_paragraphs_edit_content().
 *
 * - Saves the selected Layout Paragraphs template to the database.
 * - Saves Paragraphs items to the database with default/fallback weights.
 * - Saves Layout Paragraphs block positions to the database.
 *
 * @param array $form
 *   An associative array containing the structure of a portion of the form.
 * @param array $form_state
 *   A keyed array containing the current state of the form.
 *
 * @return void
 *
 * @see layout_paragraphs_edit_content()
 */
function layout_paragraphs_edit_content_submit($form, &$form_state) {
  // @todo: Ensure paragraph items are saved regularly into this field's
  // database table as well as saving the layout_paragraphs_blocks table. We
  // need to make sure the data remains in the field in the usual way so that
  // if Layout UI is disabled, the data is still there.
  $module = 'layout_paragraphs';
  $lp_id = 0;
  $layout_template = $form_state['values']['layout_paragraphs_templates'];
  $field_name = $form['layout_paragraphs']['#field']['field_name'];
  $entity_type = $form['#entity_type'];
  $bundle = $form['#bundle'];
  $entity = $form['layout_paragraphs']['#entity'];
  $entity_id = $entity->id() ?? 0;
  if (!empty($entity_id)) {
    // Update existing entity.
    // Save new positions to the tempstore.
    $layout_name = _layout_paragraphs_get_layout_name($form['layout_paragraphs']['#instance'], $entity_id);
    $layout = layout_paragraphs_instance_tempstore_load($layout_name);
    $layout->locked = FALSE;
    // @todo: Do we need to consider deleted Paragraphs here?
    $layout->positions = [];
    foreach ($form_state['values']['content']['positions'] as $region => $uuids_string) {
      if (!empty($uuids_string)) {
        $layout->positions[$region] = array_filter(explode(',', $uuids_string));
      }
    }
    layout_paragraphs_set_layout_tempstore($layout);
    // Save Paragraphs block/field data to the database.
    $langcode = $form['langcode']['#value'];
    $form_state['values']['content']['layout_name'] = $layout_name;
    $values = layout_paragraphs_store_field_data($form['layout_paragraphs']['#instance'], $layout_template, $entity_id);
    $form_state['values'][$field_name][$langcode] = $values;
  }
  else {
    // We need to handle insertion of new entities yet...
    // 1. The URLs on the layout have a blank where the layout name should be,
    //    e.g. when trying to add a Paragraphs block.
    // 2. Saving the entity fires this submit hook before the entity ID is set.
    if ($entity_type == 'taxonomy_term') {
      // Seems to require a special case based on how and when terms get saved.
      $entity_id = $form_state['values']['tid'] ?? 0;
    }

    $form_state['values']['content']['layout_instance_tempstore_id'] = $form_state['layout_paragraphs']['#entity_tempstore_id'];
    $insert_data = [
      'field_instance' => $form['layout_paragraphs']['#instance'],
      'layout_template' => $layout_template,
    ];
    tempstore_set('layout_paragraphs_field', $form_state['layout_paragraphs']['#entity_tempstore_id'], $insert_data, 60 * 60 * 24 * 7);
    return;
  }
}
