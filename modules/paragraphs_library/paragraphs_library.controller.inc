<?php

class ParagraphsLibraryItemController extends EntityPlusController {

  /**
   * This method is called when we use entity_create(). It provides defaults
   * for the entity properties.
   */
  public function create(array $values = array()) {
    global $user;
    $values += array(
      'title' => '',
      'created' => REQUEST_TIME,
      'changed' => REQUEST_TIME,
      'uid' => $user->uid,
    );
    return parent::create($values);
  }

  /**
   * This method is called when using Entity::view() or entity_view(). It builds
   * the render array. We provide
   */
  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {
    $wrapper = entity_metadata_wrapper('paragraphs_library_item', $entity);

    $build = parent::buildContent($entity, $view_mode, $langcode, $content);
    $build['#theme'] = 'paragraphs_library_item';
    return $build;
  }
}

class ParagraphsLibraryTypeController extends EntityPlusControllerExportable {

  /**
   * This method is called when we use entity_create(). It provides defaults
   * for the entity properties.
   */
   public function create(array $values = array()) {
    $values += array(
      'label' => '',
      'description' => '',
    );
    return parent::create($values);
  }

  /**
   * Save ParagraphsLibrary Type.
   */
  public function save($entity, ?DatabaseTransaction $transaction = NULL) {
    // Rebuild menu registry. We do not call menu_rebuild directly, but set
    // state that indicates rebuild in the end.
    state_set('menu_rebuild_needed', TRUE);
    return parent::save($entity, $transaction);
  }
}
