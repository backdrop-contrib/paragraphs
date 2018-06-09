<?php
/**
 * @file
 * Entity controller for the paragraphs entity.
 */

/**
 * Class ParagraphsItemController
 */
class ParagraphsItemController extends EntityDatabaseStorageController  {

  /**
   * Implements EntityStorageControllerInterface::save().
   */
  public function save(EntityInterface $entity) {
    $transaction = db_transaction();
    try {
      // Load the stored entity, if any.
      if (!$entity->isNew() && !isset($entity->original)) {
        $entity->original = entity_load_unchanged($this->entityType, $entity->id());
      }

      $this->preSave($entity);
      $this->invokeHook('presave', $entity);

      if (!$entity->isNew()) {
        $return = backdrop_write_record($this->entityInfo['base table'], $entity, $this->idKey);
        $this->resetCache(array($entity->{$this->idKey}));
        $this->postSave($entity, TRUE);
        $this->invokeHook('update', $entity);
      }
      else {
        $return = backdrop_write_record($this->entityInfo['base table'], $entity);
        // Reset general caches, but keep caches specific to certain entities.
        $this->resetCache(array());
        $this->postSave($entity, FALSE);
        $this->invokeHook('insert', $entity);
      }

      if ($this->revisionKey) {
        $this->saveRevision($entity);
      }


      // Ignore slave server temporarily.
      db_ignore_slave();
      unset($entity->is_new);
      unset($entity->original);

      return $return;
    }
    catch (Exception $e) {
      $transaction->rollback();
      watchdog_exception($this->entityType, $e);
      throw new EntityStorageException($e->getMessage(), (int) $e->getCode(), $e);
    }
  }

  /**
   * Saves a new revision.
   *
   * @param ParagraphsItem $entity
   *   The paragraphs item entity whose revision should be saved.
   */
  protected function saveRevision(EntityInterface $entity) {
    $record = clone $entity;

    if (empty($entity->{$this->revisionKey}) || !empty($entity->revision)) {
      backdrop_write_record($this->revisionTable, $record);
      db_update($this->entityInfo['base table'])
        ->fields(array($this->revisionKey => $record->{$this->revisionKey}))
        ->condition($this->idKey, $entity->{$this->idKey})
        ->execute();
    }
    else {
      backdrop_write_record($this->revisionTable, $record, $this->revisionKey);
    }
    // Make sure to update the new revision key for the entity.
    $entity->{$this->revisionKey} = $record->{$this->revisionKey};
  }

}
