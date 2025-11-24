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

    $field_name = $field['field_name'];
    $langcode = (!empty($langcode)) ? $langcode : LANGUAGE_NONE;

    $form['#attached']['js'][] = backdrop_get_path('module', 'layout') . '/js/layout.admin.js';
    $form['layout_paragraphs'] = array(
      '#type' => 'container',
    );
    $entity_type = $element['#entity_type'];
    $entity_id = (!empty($element['#entity'])) ? $element['#entity']->id() : 0;
    if (empty($entity_id) && !empty($form_state['input']['content']['positions'])) {
      // We need to check if content is added in the form state, to try to
      // preserve the layout that was being edited.
      $entity_id = _layout_paragraphs_get_layout_id_from_block_tempstore($form_state['input']['content']['positions']);
    }
    if (empty($entity_id)) {
      $entity_id = $form_state['layout_paragraphs']['tempstore_id'] ?? $form_state[$entity_type . '_tempstore_id'] ?? '';
    }
    $layout_name = _layout_paragraphs_get_layout_name($instance, $entity_id);
    $expire = REQUEST_TIME + settings_get('form_cache_expiration', config_get('system.core', 'form_cache_expiration'));
    // Save the current edit path to tempstore, so we can return here after
    // ajax operations.
    $edit_path = (is_numeric($entity_id)) ? base_path() . current_path() : NULL;
    tempstore_set('layout_paragraphs:edit_path', $layout_name, $edit_path, $expire);
    $form['layout_paragraphs']['#field'] = $field;
    $form['layout_paragraphs']['#instance'] = $instance;
    $form['layout_paragraphs']['#entity'] = $element['#entity'];
    $form_state['layout_paragraphs']['tempstore_id'] = $entity_id;
    $layout_name = _layout_paragraphs_get_layout_name($instance, $entity_id);
    $layout = layout_paragraphs_get_layout_tempstore($layout_name);
    if (empty($layout)) {
      $layout = layout_paragraphs_layout_load($layout_name, TRUE, 'database');
      layout_paragraphs_set_layout_tempstore($layout);
    }
    if (empty($entity_id) || !$layout) {
      $layout = layout_paragraphs_layout_load($layout_name);
      $layout->locked = FALSE;
    }

    $template_options = _layout_paragraphs_layout_get_available_templates($layout_name);
    // Existing entities will have an active template that is not counted in
    // available templates. New entities will not have an active template yet.
    if (count($template_options['available']) >= 1) {
      $form['layout_paragraphs']['select_template']['options'] = [
        '#type' => 'container',
        '#attributes' => array(
          'class' => array('layout-paragraphs--template-options'),
        ),
      ];
      if (is_numeric($entity_id)) {
        $form['layout_paragraphs']['select_template']['options']['reset_link'] = array(
          '#type' => 'link',
          '#title' => t('Reset'),
          '#href' => 'layout_paragraphs/reset/' . $layout->name,
          '#limit_validation_errors' => array(),
          '#weight' => 100,
          '#attributes' => array(
            'class' => array(
              'use-ajax',
            ),
            'data-dialog' => 'true',
            'data-dialog-options' => json_encode(array('dialogClass' => 'layout-paragraphs-dialog')),
          ),
        );
        $form['layout_paragraphs']['select_template']['options']['change_link'] = array(
          '#type' => 'link',
          '#title' => t('Change layout template'),
          '#href' => 'layout_paragraphs/template_change/' . $layout->name,
          '#limit_validation_errors' => array(),
          '#weight' => 100,
          '#attributes' => array(
            'class' => array(
              'use-ajax',
            ),
            'data-dialog' => 'true',
            'data-dialog-options' => json_encode(array('dialogClass' => 'layout-paragraphs-dialog')),
          ),
        );
      }
      elseif (count($template_options['available']) > 1) {
        // New entity, we ask them to save first before changing template if
        // there are more than one choice.
        $form['layout_paragraphs']['select_template']['options']['new_action'] = array(
          '#type' => 'container',
        );
        $form['layout_paragraphs']['select_template']['options']['new_action']['save_now'] = array(
          '#type' => 'submit',
          '#value' => t('Save now'),
          '#submit' => array('layout_paragraphs_new_content_submit'),
          '#weight' => 100,
          '#attributes' => array(
            'class' => array(
              'layout-paragraphs--save-now',
              'button--small',
            ),
          ),
        );
        $form['layout_paragraphs']['select_template']['options']['change_notice'] = array(
          '#markup' => '<div class="description"><p>' . t('Note: other layout templates are available. You can add content below, and you will be able to change the layout template after saving for the first time, if you wish.') . '</p></div>',
        );
      }
    }
    $template_name = $template_options['active']->name ?? reset($template_options['available'])->name;
    $layout->layout_template = $template_name;
    $form['layout_paragraphs']['select_template']['layout_paragraphs_template'] = array(
      '#type' => 'hidden',
      '#title' => t('Layout template'),
      '#value' => $template_name,
      '#required' => TRUE,
      '#attributes' => array(
        'id' => array('layout-paragraphs--current-template'),
      ),
    );
    $layout_form = $layout_form_state = [];
    $layout_content = layout_paragraphs_content_form($layout_form, $layout_form_state, $layout, 'paragraphs_editor');
    $form['layout_paragraphs']['layout'] = $layout_content;
    $form['layout_paragraphs']['element'] = $element;
    // $form['layout_paragraphs']['entity_tempstore_id'] = $form['layout_paragraphs']['#entity_tempstore_id'] ?? '';
    if ($entity_type == 'taxonomy_term') {
      // @todo: Figure out why this seems to be needed for terms.
      // Maybe because the submit handler is not explicitly set in taxonomy_form_term?
      // Paragraphs don't seem to be saving on new terms, unless you save the term,
      // then edit it again. Maybe tempstore needs to be set earlier?
      $form['#submit'] = [];
      $form['#submit'][] = 'taxonomy_form_term_submit';
    }
    $form['#validate'][] = 'layout_paragraphs_edit_content_validate';
    $form['#submit'][] = 'layout_paragraphs_edit_content_submit';

  }
}

/**
 * Submit handler to save a new entity and continue editing.
 */
function layout_paragraphs_new_content_submit($form, &$form_state) {
  $submit_functions = $form['actions']['submit']['#submit'] ?? [];
  foreach ($submit_functions as $function) {
    if ($function != 'layout_paragraphs_new_content_submit' && function_exists($function)) {
      $function($form, $form_state);
    }
  }
  // Redirect to the edit form for the new entity.
  $entity_type = $form['#entity_type'];
  $entity_id = $form['#' . $entity_type]->id();
  $entity_uri = $form['#' . $entity_type]->uri();
  if (backdrop_valid_path($entity_uri['path'] . '/edit')) {
    $edit_path = $entity_uri['path'] . '/edit';
  }
  elseif (backdrop_valid_path($entity_type . '/' . $entity_id . '/edit')) {
    $edit_path = $entity_type . '/' . $entity_id . '/edit';
  }
  if (!empty($edit_path)) {
    $form_state['redirect'] = $edit_path;
  }
}

/**
 * Helper to determine the available layouts for a given layout name, based on
 * looking up the field instance settings and defaults.
 *
 * @param string $layout_name
 *   The layout name.
 *
 * @return array
 *   An array of available templates, with the following keys:
 *    - 'active' (LayoutFlexibleTemplate): The active template.
 *    - 'available' (array): An array of other available templates
 *      (LayoutFlexibleTemplate objects), keyed by template machine name.
 */
function _layout_paragraphs_layout_get_available_templates($layout_name) {
  $template_options = [
    'active' => [
      'name' => 'default',
    ],
    'available' => [],
  ];
  $templates = [];
  $all_templates = layout_flexible_template_load_all('paragraphs');
  $instance = _layout_paragraphs_get_field_instance($layout_name);
  $layout = layout_paragraphs_get_layout_tempstore($layout_name);
  if (!empty($instance)) {
    $templates = $instance['widget']['settings']['available_layout_templates'];
  }
  $default_template = $layout->layout_template ?? 'default';
  if (!empty($templates)) {
    foreach ($templates as $template_name) {
      if (!empty($all_templates[$template_name])) {
        if ($template_name == $default_template) {
          $template_options['active'] = $all_templates[$template_name];
        }
        else {
          $template_options['available'][$template_name] = $all_templates[$template_name];
        }
      }
    }
  }

  return $template_options;
}

/**
 * Validate the layout settings form.
 */
function layout_paragraphs_edit_content_validate($form, &$form_state) {
  /* @var Layout $layout */
  $layout = layout_paragraphs_get_layout_tempstore($form['layout_paragraphs']['layout']['#layout']->name);
  if (!$layout) {
    $layout = $form['layout_paragraphs']['layout']['#layout'];
  }

  // Move the blocks around in tempstore as per the submitted positions. This
  // is especially needed for new entity insertion when form_state is no longer
  // available.
  $layout->positions = [];
  foreach ($form_state['values']['content']['positions'] as $region => $uuid_string) {
    $layout->positions[$region] = array_filter(explode(',', $uuid_string));
  }
  layout_paragraphs_set_layout_tempstore($layout);

  // Ensure that no block is positioned more than once. This is merely a sanity
  // check, it should not occur within the normal interface.
  $all_uuids = array();
  foreach ($layout->positions as $uuids) {
    foreach ($uuids as $uuid) {
      if (in_array($uuid, $all_uuids)) {
        $block_title = (is_object($layout->content[$uuid])) ? $layout->content[$uuid]->getAdminTitle() : $uuid;
        watchdog('layout_paragraphs', t('The "@block" item has incorrectly been positioned multiple times. This may indicate an incorrectly saved layout configuration.', array('@block' => $block_title)));
      }
      $all_uuids[] = $uuid;
    }
  }

  // Let the layout do any specific validations.
  $layout->formValidate($form, $form_state);
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
  $layout_template = $form['layout_paragraphs']['layout']['#layout']->layout_template;
  $entity_type = $form['#entity_type'];
  $entity = $form_state[$entity_type];
  $entity->revision = $form_state['values']['revision'] ?? $entity->revision;
  $entity_id = $entity->id() ?? 0;
  if (!empty($entity_id)) {
    // Update existing entity.
    // Save new positions to the tempstore.
    $layout_name = _layout_paragraphs_get_layout_name($form['layout_paragraphs']['#instance'], $entity_id);
  }
  else {
    $tempstore_id = _layout_paragraphs_get_layout_id_from_block_tempstore($form_state['input']['content']['positions']);
    if (empty($tempstore_id)) {
      $tempstore_id = $form_state['layout_paragraphs']['tempstore_id'] ?? '';
    }
    $layout_name = _layout_paragraphs_get_layout_name($form['layout_paragraphs']['#instance'], $tempstore_id);

    $insert_data = [
      'field_instance' => $form['layout_paragraphs']['#instance'],
      'layout_template' => $layout_template,
    ];
    tempstore_set('layout_paragraphs_field', $tempstore_id, $insert_data, 60 * 60 * 24 * 7);
  }
  $form_state['values']['content']['layout_name'] = $layout_name;
}
