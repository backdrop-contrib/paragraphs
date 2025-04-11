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
      '#title' => t('Layout templates'),
      '#description' => t('Which layout template(s) should be available for this Paragraphs field?'),
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
      // return paragraphs_field_multiple_value_form($field, $instance, $langcode, $items, $form, $form_state, $delta, $element);
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
 *  The language associated with $items.
 * @param array $items
 *  Array of default values for this field.
 * @param int $delta
 *  The order of this item in the array of subelements (0, 1, 2, etc).
 * @param array $element
 *  A form element array containing basic properties for the widget.
 *
 * @see layout_paragraphs_menu()
 * @see layout_paragraphs_edit_content_validate() @todo
 * @see layout_paragraphs_edit_content_submit() @todo
 * @ingroup forms
 */
function layout_paragraphs_edit_content(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
  if ($instance['widget']['type'] == 'layout_paragraphs') {
    form_load_include($form_state, 'inc', 'layout_paragraphs', 'layout_paragraphs.admin');
    form_load_include($form_state, 'inc', 'layout_paragraphs', 'layout_paragraphs.context.admin');
    form_load_include($form_state, 'inc', 'layout', 'layout.admin');

    $form['#attached']['js'][] = backdrop_get_path('module', 'layout') . '/js/layout.admin.js';
    // $form['#attached']['css'][] = backdrop_get_path('module', 'mini_layouts') . '/css/mini-layouts.admin.css';
    $form['layout_paragraphs'] = array(
      '#type' => 'container',
      '#attributes' => array(
        'id' => array('ajax-layout-paragraphs-templates'),
      ),
    );
    $form['layout_paragraphs']['#field'] = $field;
    $form['layout_paragraphs']['#instance'] = $instance;
    $all_templates = layout_flexible_template_load_all('paragraphs');
    $available_templates = $instance['widget']['settings']['available_layout_templates'];
    $entity_type = $instance['entity_type'];
    $entity_id = $form["#$entity_type"]->id();
    if (!empty($form_state['layout_paragraphs']['select_template']['layout_paragraphs_templates'])) {
      $default_template = $form_state['layout_paragraphs']['select_template']['layout_paragraphs_templates'];
    }
    else {
      $default_template = _layout_paragraphs_determine_layout_template($entity_id, $instance['entity_type'], $instance['bundle'], $instance['field_name'], $available_templates);
    }
    $active_templates = 0;
    foreach ($available_templates as $template_name => $template_status) {
      if ($template_status != 0) {
        $active_templates++;
        break;
      }
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

    $id = (!empty($form_state[$instance['entity_type']]->id())) ? '.' . $form_state[$instance['entity_type']]->id() : '';
    $name = $instance['entity_type'] . '.' . $instance['bundle'] . '.' . $instance['field_name'] . $id;
    $layout = layout_paragraphs_layout_load($name);

    $layout_form = $layout_form_state = [];
    $form['layout_paragraphs']['layout'] = layout_paragraphs_content_form($layout_form, $layout_form_state, $layout, 'paragraphs_editor');

    $form['actions']['submit']['#submit'][] = 'layout_paragraphs_edit_content_submit';
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
 *  An associative array containing the structure of a portion of the form.
 * @param array $form_state
 *  A keyed array containing the current state of the form.
 *
 * @return void
 *
 * @see layout_paragraphs_edit_content()
 */
function layout_paragraphs_edit_content_submit($form, &$form_state) {
  $module = 'layout_paragraphs';
  $lp_id = 0;
  $layout_template = $form_state['values']['layout_paragraphs_templates'];
  $field_name = $form['layout_paragraphs']['#field']['field_name'];
  $entity_type = $form['#entity_type'];
  $bundle = $form['#bundle'];
  $entity = $form["#$entity_type"];
  $entity_id = $entity->id();
  // Check if this entity already has a layout_paragraphs entry.
  $result = db_select('layout_paragraphs', 'lp')
    ->fields('lp')
    ->condition('lp.entity_id', $entity_id)
    ->condition('lp.entity_type', $entity_type)
    ->condition('lp.bundle', $bundle)
    ->condition('lp.field_name', $field_name)
    ->execute()
    ->fetchAssoc();

  if (!empty($result)) {
    $lp_id = $result['id'];
  }
  elseif (!empty($entity_id)) {
    $lp_id = layout_paragraphs_entity_template_insert($entity_id, $entity_type, $bundle, $field_name, $layout_template);
  }

  foreach ($form_state['values']['content']['positions'] as $region_id => $uuid_string) {
    $uuids = explode(',', $uuid_string);
    foreach ($uuids as $block_index => $uuid) {
      if (!empty($uuid)) {
        $block = tempstore_get('layout_paragraphs_block', $uuid);
        // @todo: Error: Cannot access protected property BlockParagraphs::$paragraphs_item
        // Temporarily set it to public; decide if this is what we actually want.
        $block->paragraphs_item->setHostEntity($entity_type, $entity);
        $block->paragraphs_item->being_edited = FALSE;
        $block->paragraphs_item->save(TRUE);
        tempstore_clear('layout_paragraphs_block', $uuid);
        // @todo: Consider revisions here. Deeper testing needed for certain.

        $block_data = json_encode([
          'paragraphs_item_id' => $block->paragraphs_item->item_id,
          'layout_name' => _layout_paragraphs_get_layout_name($form['layout_paragraphs']['#instance'], $entity_id),
        ]);
        $block_info = [
          'lp_id' => $lp_id,
          'uuid' => $uuid,
          'region_id' => $region_id,
          'block_index' => $block_index,
          'block_data' => $block_data,
        ];
        $exists = db_query('SELECT count(uuid) FROM {layout_paragraphs_blocks} WHERE uuid = :uuid', array(':uuid' => $uuid))->fetchField();
        $primary_key = ($exists) ? ['uuid'] : [];
        backdrop_write_record('layout_paragraphs_blocks', $block_info, $primary_key);
      }
    }
  }

  // layout_clear_layout_tempstore?
}
