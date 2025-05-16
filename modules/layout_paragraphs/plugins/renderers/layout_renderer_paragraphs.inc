<?php
/**
 * The renderer for the Paragraphs template editor.
 */
class LayoutRendererParagraphs extends LayoutRendererFlexible {
  /**
   * A stub Layout object required by the theme layer.
   *
   * @var Layout
   */
  var $layout;

  /**
   * Deprecated. Now replaced by $layout_template_info.
   *
   * @var NULL
   * @deprecated since 1.29.0
   */
  var $layout_info;

  /**
   * The information from hook_layout_template_info() for the layout template.
   *
   * @var array
   */
  var $layout_template_info;

  /**
   * The complete rendered editor.
   *
   * @var array
   */
  var $rendered_editor = array();

  /**
   * TRUE if this renderer is rendering in administrative mode.
   *
   * @var bool
   */
  var $admin = TRUE;

  /**
   * Include rendered HTML prior to the layout.
   *
   * @var string
   */
  var $prefix = '';

  /**
   * Include rendered HTML after the layout.
   *
   * @var string
   */
  var $suffix = '';

  /**
   * An array of dropbutton links to administrative actions on editor rows.
   *
   * @var array
   */
  var $region_buttons = array();

  /**
   * Construct the object that will be used to render the editor.
   *
   * @param Layout $layout
   *   The stub layout object required by the theme layer.
   * @param array $renderer_plugin
   *   The definition of the renderer plugin.
   */
  function __construct(Layout $layout, array $renderer_plugin = array()) {
    $template_id = $layout->layout_template;

    $template_info = layout_paragraphs_get_layout_template_info($template_id);
    $template_data = layout_paragraphs_tempstore_load($template_id);

    foreach ($template_data->rows as $position => $info) {
      $layout->positions[$position] = array();
      $template_info['regions'][$position] = $position;
    }

    $this->admin = TRUE;
    $this->layout = &$layout;
    $this->layout_template_info = $template_info;
  }

  /**
   * Render the entire editor HTML and attach meta data.
   *
   * @return string
   *   Themed & rendered HTML output.
   */
  function render() {
    $this->addMeta();

    $output = '<div id="layout-paragraphs-edit-main">';
    $output .= $this->renderEditor();
    $output .= '</div>';

    return $output;
  }

  /**
   * Perform display-level render operations.
   *
   * This method renders the flexible editor rows, passes this to the flexible
   * template's theme callback, and returns the rendered HTML.
   *
   * @return string
   *   The HTML string representing the entire rendered, themed template editor.
   */
  function renderEditor() {
    $this->renderRowButtons();

    module_load_include('inc', 'layout', 'layout.theme');
    if (empty($this->layout_template_info['template'])) {
      $theme = 'layout__paragraphs';
    }
    else {
      $theme = str_replace('-', '_', $this->layout_template_info['template']);
    }

    $this->rendered_editor = theme($theme, array(
      'content' => array(),
      'settings' => array(),
      'layout' => $this->layout,
      'layout_template_info' => $this->layout_template_info,
      'renderer' => $this,
      'admin' => $this->admin,
    ));
    return $this->prefix . $this->rendered_editor . $this->suffix;
  }

  /**
   * Attach page metadata.
   */
  function addMeta() {
    backdrop_add_library('system', 'backdrop.ajax');
    backdrop_add_library('system', 'ui.sortable');
    backdrop_add_js(backdrop_get_path('module', 'layout') . '/js/layout.admin.js');
    backdrop_add_css(backdrop_get_path('module', 'layout') . '/css/layout.admin.css');
    backdrop_add_css(backdrop_get_path('module', 'layout') . '/css/layout.flexible.admin.css');
  }

  /**
   * Render row dropbuttons.
   */
  function renderRowButtons() {
    foreach ($this->layout_template_info['regions'] as $region_id => $title) {
      $this->region_buttons[$region_id] = $this->getRowButtons($region_id);
    }

    return $this->region_buttons;
  }

  /**
   * Render the links to display when editing a region.
   */
  protected function getRowButtons($region_id) {
    $links = array();
    $links['configure'] = array(
      'title' => t('Configure'),
      'href' => 'admin/structure/paragraphs/layouts/' . $this->layout->layout_template . '/row/' . $region_id . '/configure',
      'attributes' => array(
        'class' => array('use-ajax'),
        'data-dialog' => TRUE,
        'data-dialog-options' => json_encode(array('dialogClass' => 'layout-dialog')),
      ),
    );
    $links['delete'] = array(
      'title' => t('Delete row'),
      'href' => 'admin/structure/paragraphs/layouts/' . $this->layout->layout_template . '/row/' . $region_id . '/delete',
      'query' => array(
        'token' => backdrop_get_token('layout-region-' . $region_id),
      ),
      'attributes' => array(
        'class' => array('use-ajax'),
      ),
    );

    $dropbutton = array(
      '#type' => 'dropbutton',
      '#links' => $links,
    );

    return backdrop_render($dropbutton);
  }
}
