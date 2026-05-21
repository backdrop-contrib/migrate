<?php

/**
 * @file
 * Support for node destinations.
 */

/**
 * @todo
 * Make sure this works with updates, explicit destination keys
 */

/**
 * Destination class implementing migration into nodes.
 */
class MigrateDestinationNode extends MigrateDestinationEntity {

  protected $bypassDestIdCheck = FALSE;

  /**
 *
 */
  static public function getKeySchema() {
    return array(
      'nid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'description' => 'ID of destination node',
      ),
    );
  }

  /**
   * Return an options array for node destinations.
   *
   * @param string $langcode
   *   Default langcode for nodes created via this destination class.
   * @param string $text_format
   *   Default text format for nodes created via this destination class.
   */
  static public function options($langcode, $text_format) {
    return compact('langcode', 'text_format');
  }

  /**
   * Basic initialization
   *
   * @param string $bundle
   *   A.k.a. the content type (page, article, etc.) of the node.
   * @param array $options
   *   Options applied to nodes.
   */
  public function __construct($bundle, array $options = array()) {
    parent::__construct('node', $bundle, $options);
  }

  /**
   * Returns a list of fields available to be mapped for the node type (bundle)
   *
   * @param Migration $migration
   *   Optionally, the migration containing this destination.
   *
   * @return array
   *   Keys: machine names of the fields (to be passed to addFieldMapping)
   *   Values: Human-friendly descriptions of the fields.
   */
  public function fields($migration = NULL) {
    $fields = array();
    // First the core (node table) properties.
    $fields['nid'] = t('Node: <a href="@doc">Existing node ID</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-node-fields'));
    $node_type = node_type_load($this->bundle);
    if ($node_type->has_title) {
      $fields['title'] = t('Node: <a href="@doc">',
          array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-node-fields'))
        . $node_type->title_label . '</a>';
    }
    $fields['uid'] = t('<a href="@doc">Authored by (uid)</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-node-fields'));
    $fields['created'] = t('<a href="@doc">Created timestamp</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-node-fields'));
    $fields['changed'] = t('<a href="@doc">Modified timestamp</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-node-fields'));
    $fields['status'] = t('<a href="@doc">Published</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-node-fields'));
    $fields['promote'] = t('<a href="@doc">Promoted to front page</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-node-fields'));
    $fields['sticky'] = t('<a href="@doc">Sticky at top of lists</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-node-fields'));
    $fields['revision'] = t('<a href="@doc">Create new revision</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-node-fields'));
    $fields['log'] = t('<a href="@doc">Revision Log message</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-node-fields'));
    $fields['langcode'] = t('<a href="@doc">Language (fr, en, ...)</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-node-fields'));
    $fields['tnid'] = t('<a href="@doc">The translation set id for this node</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-node-fields'));
    $fields['translate'] = t('<a href="@doc">A boolean indicating whether this translation page needs to be updated</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-node-fields'));
    $fields['revision_uid'] = t('<a href="@doc">Modified (uid)</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-node-fields'));
    $fields['is_new'] = t('Option: <a href="@doc">Indicates a new node with the specified nid should be created</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-node-fields'));

    // Then add in anything provided by handlers.
    $fields += migrate_handler_invoke_all('Entity', 'fields', $this->entityType, $this->bundle, $migration);
    $fields += migrate_handler_invoke_all('Node', 'fields', $this->entityType, $this->bundle, $migration);

    return $fields;
  }

  /**
   * Delete a batch of nodes at once.
   *
   * @param $nids
   *   Array of node IDs to be deleted.
   */
  public function bulkRollback(array $nids) {
    migrate_instrument_start('node_delete_multiple');
    $this->prepareRollback($nids);
    node_delete_multiple($nids);
    $this->completeRollback($nids);
    migrate_instrument_stop('node_delete_multiple');
  }

  /**
   * Import a single node.
   *
   * @param $node
   *   Node object to build. Prefilled with any fields mapped in the Migration.
   * @param $row
   *   Raw source data object - passed through to prepare/complete handlers.
   *
   * @return array
   *   Array of key fields (nid only in this case) of the node that was saved if
   *   successful. FALSE on failure.
   */
  public function import(stdClass $stub_node, stdClass $row) {
    // Updating previously-migrated content?
    $migration = Migration::currentMigration();
    if (isset($row->migrate_map_destid1) && !$this->bypassDestIdCheck) {
      // Make sure is_new is off.
      $stub_node->is_new = FALSE;
      if (isset($stub_node->nid)) {
        if ($stub_node->nid != $row->migrate_map_destid1) {
          throw new MigrateException(t("Incoming nid !nid and map destination nid !destid1 don't match",
            array(
              '!nid' => $stub_node->nid,
              '!destid1' => $row->migrate_map_destid1,
            )));
        }
      }
      else {
        $stub_node->nid = $row->migrate_map_destid1;
      }
      // Get the existing vid, tnid so updates don't generate notices.
      $values = db_select('node', 'n')
        ->fields('n', array('vid', 'tnid'))
        ->condition('nid', $stub_node->nid)
        ->execute()
        ->fetchAssoc();
      if (empty($values)) {
        throw new MigrateException(t("Incoming node ID !nid no longer exists",
          array('!nid' => $stub_node->nid)));
      }
      $stub_node->vid = $values['vid'];
      if (empty($stub_node->tnid)) {
        $stub_node->tnid = $values['tnid'];
      }
    }
    $old_node = (isset($stub_node->nid)) ? node_load($stub_node->nid) : NULL;
    if ($migration->getSystemOfRecord() == Migration::DESTINATION) {
      if (!isset($stub_node->nid)) {
        throw new MigrateException(t('System-of-record is DESTINATION, but no destination nid provided'));
      }
      if (empty($old_node)) {
        throw new MigrateException(t('System-of-record is DESTINATION, but node !nid does not exist',
          array('!nid' => $stub_node->nid)));
      }
      if (!isset($stub_node->created)) {
        $stub_node->created = $old_node->created;
      }
      if (!isset($stub_node->vid)) {
        $stub_node->vid = $old_node->vid;
      }
      if (!isset($stub_node->status)) {
        $stub_node->status = $old_node->status;
      }
      if (!isset($stub_node->uid)) {
        $stub_node->uid = $old_node->uid;
      }
    }

    if (!isset($stub_node->type)) {
      // Default the type to our designated destination bundle (by doing this
      // conditionally, we permit some flexibility in terms of implementing
      // migrations which can affect more than one type).
      $stub_node->type = $this->bundle;
    }

    $node = _migrate_cast_to_class($stub_node, $old_node, 'Node');
    // Set some required properties.
    if ($migration->getSystemOfRecord() == Migration::SOURCE) {
      if (empty($node->langcode)) {
        $node->langcode = $this->langcode;
      }

      // Apply defaults, allow standard node prepare hooks to fire.
      // node_object_prepare() will blow these away, so save them here and
      // stuff them in later if need be.
      if (isset($node->created)) {
        $created = MigrationBase::timestamp($node->created);
      }
      else {
        // To keep node_object_prepare() from choking.
        $node->created = REQUEST_TIME;
      }
      if (isset($node->changed)) {
        $changed = MigrationBase::timestamp($node->changed);
      }
      if (isset($node->uid)) {
        $uid = $node->uid;
      }
      if (isset($node->revision)) {
        $revision = $node->revision;
      }

      node_object_prepare($node);

      if (isset($created)) {
        $node->created = $created;
      }
      // No point to resetting $node->changed here, node_save() will overwrite it.
      if (isset($uid)) {
        $node->uid = $uid;
      }
      if (isset($revision)) {
        $node->revision = $revision;
      }
    }

    // Invoke migration prepare handlers.
    $this->prepare($node, $row);

    if (!isset($node->revision)) {
      $node->revision = 0; // Saves disk space and writes. Can be overridden.
    }

    // // Trying to update an existing node.
    // if ($migration->getSystemOfRecord() == Migration::DESTINATION) {
    //   // Incoming data overrides existing data, so only copy non-existent fields.
    //   foreach ($old_node as $field => $value) {
    //     // An explicit NULL in the source data means to wipe to old value (i.e.,
    //     // don't copy it over from $old_node)
    //     if (property_exists($node, $field) && $node->{$field} === NULL) {
    //       // Ignore this field.
    //     }
    //     elseif (!isset($node->{$field})) {
    //       $node->{$field} = $old_node->{$field};
    //     }
    //   }
    // }

    if (isset($node->nid) && !(isset($node->is_new) && $node->is_new)) {
      $updating = TRUE;
    }
    else {
      $updating = FALSE;
    }

    // Make sure that if is_new is not TRUE, it is not present.
    if (isset($node->is_new) && empty($node->is_new)) {
      unset($node->is_new);
    }

    // Validate field data prior to saving.
    MigrateDestinationEntity::fieldAttachValidate('node', $node);

    migrate_instrument_start('node_save');
    node_save($node);
    migrate_instrument_stop('node_save');

    if (isset($node->nid)) {
      if ($updating) {
        $this->numUpdated++;
      }
      else {
        $this->numCreated++;
      }

      // Unfortunately, http://drupal.org/node/722688 was not accepted, so fix
      // the changed timestamp.
      if (isset($changed)) {
        db_update('node')
          ->fields(array('changed' => $changed))
          ->condition('nid', $node->nid)
          ->execute();
        $node->changed = $changed;
      }

      // Potentially fix uid and timestamp in node_revisions.
      $query = db_update('node_revision')
        ->condition('vid', $node->vid);
      if (isset($changed)) {
        $fields['timestamp'] = $changed;
      }
      $revision_uid = isset($node->revision_uid) ? $node->revision_uid : $node->uid;
      if ($revision_uid != $GLOBALS['user']->uid) {
        $fields['uid'] = $revision_uid;
      }
      if (!empty($fields)) {
        // We actually have something to update.
        $query->fields($fields);
        $query->execute();
        if (isset($changed)) {
          $node->timestamp = $changed;
        }
      }
      $return = array($node->nid);
    }
    else {
      $return = FALSE;
    }

    $this->complete($node, $row);
    return $return;
  }
}

/**
 * Allows you to import revisions.
 *
 * Adapted from
 * http://www.darrenmothersele.com/blog/2012/07/16/migrating-node-revisions-backdrop-7/
 *
 * Class MigrateDestinationNodeRevision
 *
 * @author darrenmothersele
 * @author cthos
 */
class MigrateDestinationNodeRevision extends MigrateDestinationNode {

  /**
   * Basic initialization.
   *
   * @see parent::__construct
   *
   * @param string $bundle
   *   A.k.a. the content type (page, article, etc.) of the node.
   * @param array $options
   *   Options applied to nodes.
   */
  public function __construct($bundle, array $options = array()) {
    parent::__construct($bundle, $options);

    $this->bypassDestIdCheck = TRUE;
  }

  /**
   * Get key schema for the node revision destination.
   *
   * @see MigrateDestination::getKeySchema
   *
   * @return array
   *   Returns the key schema.
   */
  static public function getKeySchema() {
    return array(
      'vid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'description' => 'ID of destination node revision',
      ),
    );
  }

  /**
   * Returns additional fields on top of node destinations.
   *
   * @param string $migration
   *   Active migration.
   *
   * @return array
   *   Fields.
   */
  public function fields($migration = NULL) {
    $fields = parent::fields($migration);
    $fields['vid'] = t('Node: <a href="@doc">Revision (vid)</a>', array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-node-revision-fields'));
    return $fields;
  }

  /**
   * Rolls back any versions that have been created.
   *
   * @param array $vids
   *   Version ids to roll back.
   */
  public function bulkRollback(array $vids) {
    migrate_instrument_start('revision_delete_multiple');
    $this->prepareRollback($vids);
    $nids = array();
    foreach ($vids as $vid) {
      if ($revision = node_load(NULL, $vid)) {
        db_delete('node_revision')
          ->condition('vid', $revision->vid)
          ->execute();
        module_invoke_all('node_revision_delete', $revision);
        field_attach_delete_revision('node', $revision);
        $nids[$revision->nid] = $revision->nid;
      }
    }
    $this->completeRollback($vids);
    foreach ($nids as $nid) {
      $vid = db_select('node_revision', 'nr')
        ->fields('nr', array('vid'))
        ->condition('nid', $nid, '=')
        ->execute()
        ->fetchField();
      if (!empty($vid)) {
        db_update('node')
          ->fields(array('vid' => $vid))
          ->condition('nid', $nid, '=')
          ->execute();
      }
    }
    migrate_instrument_stop('revision_delete_multiple');
  }

  /**
   * Overridden import method.
   *
   * This is done because parent::import will return the nid of the newly
   * created nodes. This is bad since the migrate_map_* table will have
   * nids instead of vids, which could cause a nightmare explosion on
   * rollback.
   *
   * @param stdClass $node
   *   Populated entity.
   *
   * @param stdClass $row
   *   Source information in object format.
   *
   * @return array|bool
   *   Array with newly created vid, or FALSE on error.
   *
   * @throws MigrateException
   */
  public function import(stdClass $stub_node, stdClass $row) {
    // We're importing revisions, this should be set.
    $stub_node->revision = 1;

    if (empty($stub_node->nid)) {
      throw new MigrateException(t('Missing incoming nid.'));
    }

    $original_updated = $this->numUpdated;

    parent::import($stub_node, $row);

    // Reset num updated and increment created since new revision is always an update.
    $this->numUpdated = $original_updated;
    $this->numCreated++;

    if (empty($stub_node->vid)) {
      return FALSE;
    }

    return array($stub_node->vid);
  }
}
