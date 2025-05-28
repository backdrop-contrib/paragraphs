<?php
/**
 * @file
 * Paragraphs Library Item controller information.
 */

/**
 * Define a controller class for the Paragraphs Library Item.
 */
class ParagraphsLibraryItemController extends EntityPlusController {

  /**
   * This method is called when we use entity_create(). It provides defaults
   * for the entity properties.
   */
  public function create(array $values = []) {
    global $user;
    $values += [
      'title' => '',
      'created' => REQUEST_TIME,
      'changed' => REQUEST_TIME,
      'uid' => $user->uid,
    ];
    return parent::create($values);
  }

  /**
   * This method is called when using Entity::view() or entity_view(). It builds
   * the render array. We provide
   */
  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = []) {
    $wrapper = entity_metadata_wrapper('paragraphs_library_item', $entity);

    $build = parent::buildContent($entity, $view_mode, $langcode, $content);
    $build['#theme'] = 'paragraphs_library_item';
    return $build;
  }
}
