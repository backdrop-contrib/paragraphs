<?php
/**
 * @file
 * A Paragraphs block will eventually allow adding a Paragraph to content
 * via the Layout Paragraphs module.
 */

/**
 *
 */
class BlockParagraphs extends Block {

  /**
   * The Paragraphs item entity.
   *
   * @var ParagraphsItemEntity
   */
  public $paragraphs_item;

  /**
   * The field instance of the Paragraphs field.
   *
   * @var array
   */
  protected $field_instance;

  /**
   * The Paragraphs bundle information.
   *
   * @var object
   */
  protected $bundle;

  /**
   * The entity ID of the host.
   *
   * @var int
   */
  protected $host_entity_id;

  /**
   * Whether this block is orphaned.
   *
   * @var bool
   */
  protected $is_orphaned;

  /**
   * {@inheritdoc}
   */
  public function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);

    $this->settings += array(
      'admin_label' => '',
      'admin_description' => '',
    );
    $this->is_orphaned = !empty($data['orphan']);

    // $this->host_entity_id = end(preg_split('/./', $data['layout_name']));
    $this->field_instance = _layout_paragraphs_get_field_instance($data['layout_name']);
    $this->bundle = paragraphs_bundle_load($this->delta);
    if (!empty($data['paragraphs_item_id'])) {
      $this->paragraphs_item = entity_load('paragraphs_item', $data['paragraphs_item_id']);
      $this->status = $this->paragraphs_item->status;
    }
    else {
      /** @var $paragraphs_shell ParagraphsItemEntity **/
      $paragraphs_shell = entity_create('paragraphs_item', array(
        'bundle' => $this->delta,
        'field_name' => $this->field_instance['field_name'],
      ));
      $this->paragraphs_item = $paragraphs_shell;
    }
  }

  /**
   *  Sets title text on draggable block panel in Layout builder.
   */
  public function getAdminTitle() {
    $suffix = $this->is_orphaned ? ' (' . t('orphaned') . ')' : '';
    if (!empty($this->settings['admin_label'])) {
      // If a custom admin title was added in the block itself, use it first.
      return check_plain($this->settings['admin_label']) . $suffix;
    }
    else {
      // Otherwise, try for a title from the particular Paragraphs bundle.
      if (!empty($this->bundle->name)) {
        return $this->bundle->name . $suffix;
      }
    }
    if (!empty($this->field_instance['settings']['title'])) {
      // Still nothing? We'll try the general field instance title.
      return $this->field_instance['settings']['title'] . $suffix;
    }

    // Last resort: use the default Paragraphs title.
    return PARAGRAPHS_DEFAULT_TITLE . $suffix;
  }

  /**
   * {@inheritdoc}
   */
  public function getAdminPreview() {
    $display_mode = 'paragraphs_editor_preview';
    if (!empty($this->settings['admin_description'])) {
      return filter_xss($this->settings['admin_description']);
    }
    $preview_markup = entity_view($this->paragraphs_item, $display_mode);
    return render($preview_markup);
  }

  /**
   *  Paragraphs don't have front-facing titles, so this returns an empty string.
   */
  public function getTitle() {
    return '';
  }

  /**
   * Returns the rendered content of this block.
   *
   * @return string
   */
  public function getContent() {
    $display_mode = $this->field_instance['display']['default']['settings']['view_mode'];
    $content = '';
    if (!empty($this->paragraphs_item)) {
      // @todo: ensure modal admin links get added if appropriate.
      $content = $this->paragraphs_item->view($display_mode);
    }
    return $content;
  }

  /**
   * Overrides Block::getChildren().
   *
   * Return available child blocks (e.g. Paragraph types that are available).
   * Array is keyed by child_type and values are the child block_info.
   *
   * @return array|null
   */
  public function getChildren() {
    $blocks = array();
    $query_parts = explode('/', check_plain($_GET['q']));
    $layout_name = $query_parts[5];
    $field_instance = _layout_paragraphs_get_field_instance($layout_name);
    if (!empty($field_instance)) {
      $entity_info = entity_get_info($field_instance['entity_type']);
      $host_shell = entity_create($field_instance['entity_type'], array($entity_info['bundle keys']['bundle'] => $field_instance['bundle']));
      $original_element = array(
        '#entity_type' => $field_instance['entity_type'],
        '#entity' => $host_shell,
      );
      $instance_bundles = _paragraphs_get_field_instance_bundles($field_instance, $original_element);
      $all_paragraphs_blocks = layout_paragraphs_get_block_info();
      foreach ($instance_bundles['selected'] as $machine_name => $bundle) {
        if (!empty($all_paragraphs_blocks['layout_paragraphs'][$machine_name])) {
          $blocks[$machine_name] = $all_paragraphs_blocks['layout_paragraphs'][$machine_name];
        }
      }
    }
    return $blocks;
  }

  /**
   * Overrides Block::form().
   * Builds the block's configuration form.
   */
  public function form(&$form, &$form_state) {
    $form['#attached']['js'][] = backdrop_get_path('module', 'layout') . '/js/layout.admin.js';
    $this->paragraphs_item->being_edited = TRUE;

    field_attach_form('paragraphs_item', $this->paragraphs_item, $form, $form_state);

    $form['admin_label'] = array(
      '#type' => 'fieldset',
      '#title' => t('Admin label'),
      '#tree' => FALSE,
      '#collapsed' => empty($this->settings['admin_label']),
      '#states' => array(
        'collapsed' => array(
          ':input[name="reusable"]' => array('checked' => FALSE),
        ),
      ),
      '#collapsible' => TRUE,
      '#weight' => 4,
    );
    $form['admin_label']['admin_label'] = array(
      '#type' => 'textfield',
      '#default_value' => isset($this->settings['admin_label']) ? $this->settings['admin_label'] : '',
      '#title' => t('Admin label'),
      '#description' => t('Used to help identify the block in layouts while editing.'),
      '#maxlength' => 255,
    );
    $form['admin_label']['admin_description'] = array(
      '#type' => 'textfield',
      '#title' => t('Admin description'),
      '#maxlength' => 128,
      '#default_value' => isset($this->settings['admin_description']) ? $this->settings['admin_description'] : '',
      '#description' => t('This text is used only in administrative interfaces. It will not be shown to site visitors.<br />Allowed HTML tags: @tags', array('@tags' => _filter_xss_display_allowed_tags())),
      '#weight' => 4,
    );
  }

  /**
   * Validates the block form settings.
   */
  public function formValidate($form, &$form_state) {
    parent::formValidate($form, $form_state);

    field_attach_form_validate('paragraphs_item', $this->paragraphs_item, $form, $form_state);
  }

  /**
   * Submit handler to save the block form settings.
   */
  public function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    $this->settings['admin_label'] = trim($form_state['values']['admin_label']);
    $this->settings['admin_description'] = trim($form_state['values']['admin_description']);
  }
}
