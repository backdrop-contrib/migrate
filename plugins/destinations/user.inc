<?php

/**
 * @file
 * Support for user destinations.
 */

/**
 * @todo
 * Make sure this works with updates, explicit destination keys
 * Speed up password generation a ton: $conf['password_count_log2'] = 1;
 */

/**
 * Destination class implementing migration into users.
 */
class MigrateDestinationUser extends MigrateDestinationEntity {

  /**
   * Indicates whether incoming passwords are md5-encrypted - if so, we will
   * rehash them similarly to the D6->D7 upgrade path.
   *
   * @var boolean
   */
  protected $md5Passwords = FALSE;

  /**
 *
 */
  static public function getKeySchema() {
    return array(
      'uid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'description' => 'ID of destination user',
      ),
    );
  }

  /**
   * Return an options array for user destinations.
   *
   * @param string $langcode
   *   Default langcode for usrs created via this destination class.
   * @param string $text_format
   *   Default text format for users created via this destination class.
   * @param boolean $md5_passwords
   *   Set TRUE to indicate incoming passwords are md5-encrypted.
   */
  static public function options($langcode, $text_format, $md5_passwords) {
    return compact('langcode', 'text_format', 'md5_passwords');
  }

  /**
   * Basic initialization
   *
   * @param array $options
   *   Options applied to comments.
   */
  public function __construct(array $options = array()) {
    parent::__construct('user', 'user', $options);
    if (!empty($options['md5_passwords'])) {
      $this->md5Passwords = $options['md5_passwords'];
    }

    // Reduce hash count so import runs in a reasonable time (use same value as
    // the standard Backdrop 6=>Backdrop 7 upgrade path).
    global $conf;
    $conf['password_count_log2'] = 11;
  }

  /**
   * Returns a list of fields available to be mapped for users
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
    // First the core (users table) properties.
    $fields['uid'] = t('<a href="@doc">Existing user ID</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-user-fields'));
    $fields['mail'] = t('<a href="@doc">Email address</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-user-fields'));
    $fields['name'] = t('<a href="@doc">Username</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-user-fields'));
    $fields['pass'] = t('<a href="@doc">Password</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-user-fields'));
    $fields['status'] = t('<a href="@doc">Status</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-user-fields'));
    $fields['created'] = t('<a href="@doc">Registered timestamp</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-user-fields'));
    $fields['access'] = t('<a href="@doc">Last access timestamp</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-user-fields'));
    $fields['login'] = t('<a href="@doc">Last login timestamp</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-user-fields'));
    $fields['roles'] = t('<a href="@doc">Role names</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-user-fields'));
    $fields['picture'] = t('<a href="@doc">Picture</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-user-fields'));
    $fields['signature'] = t('<a href="@doc">Signature</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-user-fields'));
    $fields['signature_format'] = t('<a href="@doc">Signature format</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-user-fields'));
    $fields['timezone'] = t('<a href="@doc">Timezone</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-user-fields'));
    $fields['langcode'] = t('<a href="@doc">Language</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-user-fields'));
    $fields['theme'] = t('<a href="@doc">Default theme</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-user-fields'));
    $fields['init'] = t('<a href="@doc">Init</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-user-fields'));
    $fields['data'] = t('<a href="@doc">Data</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-user-fields'));
    $fields['is_new'] = t('Option: <a href="@doc">Indicates a new user with the specified uid should be created</a>',
      array('@doc' => 'https://github.com/backdrop-contrib/migrate/wiki/Destination-classes#user-content-user-fields'));

    // Then add in anything provided by handlers.
    $fields += migrate_handler_invoke_all('Entity', 'fields', $this->entityType, $this->bundle, $migration);
    $fields += migrate_handler_invoke_all('User', 'fields', $this->entityType, $this->bundle, $migration);

    return $fields;
  }

  /**
   * Delete a batch of users at once.
   *
   * @param $uids
   *   Array of user IDs to be deleted.
   */
  public function bulkRollback(array $uids) {
    migrate_instrument_start('user_delete_multiple');
    $this->prepareRollback($uids);
    user_delete_multiple($uids);
    $this->completeRollback($uids);
    migrate_instrument_stop('user_delete_multiple');
  }

  /**
   * Import a single user.
   *
   * @param $account
   *   Account object to build. Prefilled with any fields mapped in the
   *   Migration.
   * @param $row
   *   Raw source data object - passed through to prepare/complete handlers.
   *
   * @return array
   *   Array of key fields (uid only in this case) of the user that was saved if
   *   successful. FALSE on failure.
   */
  public function import(stdClass $stub_account, stdClass $row) {
    $migration = Migration::currentMigration();
    // Updating previously-migrated content?
    if (isset($row->migrate_map_destid1)) {
      // Make sure is_new is off.
      $stub_account->is_new = FALSE;
      if (isset($stub_account->uid)) {
        if ($stub_account->uid != $row->migrate_map_destid1) {
          throw new MigrateException(t("Incoming uid !uid and map destination uid !destid1 don't match",
            array(
              '!uid' => $stub_account->uid,
              '!destid1' => $row->migrate_map_destid1,
            )));
        }
      }
      else {
        $stub_account->uid = $row->migrate_map_destid1;
      }
    }
    if ($migration->getSystemOfRecord() == Migration::DESTINATION) {
      if (!isset($stub_account->uid)) {
        throw new MigrateException(t('System-of-record is DESTINATION, but no destination uid provided'));
      }
      $old_account = user_load($stub_account->uid, TRUE);
      if (empty($old_account)) {
        throw new MigrateException(t('System-of-record is DESTINATION, but user !uid does not exist',
          array('!uid' => $stub_account->uid)));
      }
    }
    else {
      $old_account = $stub_account;
    }

    if (!empty($stub_account->roles)) {
      if (!is_array($stub_account->roles)) {
        $stub_account->roles = array($stub_account->roles);
      }
      $stub_account->roles = backdrop_map_assoc($stub_account->roles);
    }
    if (empty($stub_account->roles) && empty($old_account->roles)) {
      $stub_account->roles = array();
    }

    $this->prepare($stub_account, $row);

    if (isset($stub_account->uid) && empty($stub_account->is_new)) {
      $updating = TRUE;
    }
    else {
      $updating = FALSE;
    }

    // While user_save is happy to see a fid in $stub_account->picture on insert,
    // when updating an existing account it wants a file object.
    if ($updating && isset($stub_account->picture) && ($fid = $stub_account->picture)) {
      $stub_account->picture = file_load($fid);
    }

    // Normalize MD5 passwords to lowercase, as generated by Drupal 6 and previous.
    if ($this->md5Passwords) {
      $stub_account->pass = backdrop_strtolower($stub_account->pass);
    }

    // If any datetime values were included, ensure that they're in timestamp format.
    if (isset($stub_account->created)) {
      $stub_account->created = MigrationBase::timestamp($stub_account->created);
    }
    if (isset($stub_account->access)) {
      $stub_account->access = MigrationBase::timestamp($stub_account->access);
    }
    if (isset($stub_account->login)) {
      $stub_account->login = MigrationBase::timestamp($stub_account->login);
    }

    // Validate field data prior to saving.
    MigrateDestinationEntity::fieldAttachValidate('user', $stub_account);

    // Cast to a user object so that user_save() will recognize it.
    $account = _migrate_cast_to_class($stub_account, $old_account, 'User');
    migrate_instrument_start('user_save');
    user_save($account);
    migrate_instrument_stop('user_save');
    if ($account) {
      if ($this->md5Passwords && !empty($account->pass)) {
        // Ape the Drupal 6 -> Drupal 7 upgrade, which encrypts the MD5 text in the
        // modern way, and marks it with a prepended U so it recognizes and fixes it
        // up at login time.
        $password = 'U' . $account->pass;
        db_update('users')
          ->fields(array('pass' => $password))
          ->condition('uid', $account->uid)
          ->execute();
      }
      // Unlike nodes and taxonomy terms, core does not automatically save an
      // alias in a user entity, we must do it ourselves.
      if (module_exists('path')) {
        if (!empty($stub_account->path['alias'])) {
          $path = array(
            'source' => 'user/' . $account->uid,
            'alias' => $stub_account->path['alias'],
          );
          migrate_instrument_start('path_save');
          path_save($path);
          migrate_instrument_stop('path_save');
        }
      }
      if ($updating) {
        $this->numUpdated++;
      }
      else {
        $this->numCreated++;
        // user_save() doesn't update file_usage on account creation, we have
        // to do it ourselves.
        if (!empty($stub_account->picture)) {
          $file = file_load($stub_account->picture);
          if (is_object($file)) {
            file_usage_add($file, 'user', 'user', $account->uid);
          }
        }
      }
      $this->complete($account, $row);
      $return = array($account->uid);
    }
    else {
      $return = FALSE;
    }
    return $return;
  }
}
/**
 *
 */
class MigrateDestinationRole extends MigrateDestinationTable {

  /**
 *
 */
  public function __construct() {
    parent::__construct('role');
  }

  /**
   * Get the key definition for the role table.
   *
   * @param $dummy
   *   PHP is picky - we need to have a parameter because MigrateDestinationTable
   *   has one.
   */
  static public function getKeySchema($dummy = NULL) {
    return array(
      'name' => array(
        'type' => 'varchar',
        'length' => 255,
        'description' => 'Name of the role',
      ),
    );
  }

  /**
   * Delete a single row.
   *
   * @param $id
   *   Primary key values.
   */
  public function rollback(array $id) {
    migrate_instrument_start('role rollback');
    $name = reset($id);
    user_role_delete($name);
    migrate_instrument_stop('role rollback');
  }

  /**
   * Import a single row.
   *
   * @param $entity
   *   Object object to build. Prefilled with any fields mapped in the Migration.
   * @param $row
   *   Raw source data object - passed through to prepare/complete handlers.
   *
   * @return array
   *   Array of key fields of the object that was saved if
   *   successful. FALSE on failure.
   */
  public function import(stdClass $entity, stdClass $row) {
    $migration = Migration::currentMigration();
    $updating = FALSE;
    // Updating previously-migrated content?
    if (isset($row->migrate_map_destid1)) {
      $updating = TRUE;
      if (isset($entity->name)) {
        if ($entity->name != $row->migrate_map_destid1) {
          throw new MigrateException(t("Incoming id !id and map destination id !destid don't match",
            array(
              '!id' => $entity->name,
              '!destid' => $row->migrate_map_destid1,
            )));
        }
      }
      else {
        $entity->name = $row->migrate_map_destid1;
      }
    }

    if ($migration->getSystemOfRecord() == Migration::DESTINATION) {
      $updating = TRUE;
      if (!isset($entity->name)) {
        throw new MigrateException(t('System-of-record is DESTINATION, but no destination id provided'));
      }

      $old_entity = user_role_load($entity->name);

      foreach ($entity as $field => $value) {
        $old_entity->{$field} = $entity->{$field};
      }
      $entity = $old_entity;
    }

    $this->prepare($entity, $row);
    user_role_save($entity);
    $this->complete($entity, $row);

    if (!empty($entity->name)) {
      $id = array($entity->name);
      if ($updating) {
        $this->numUpdated++;
      }
      else {
        $this->numCreated++;
      }
    }
    else {
      $id = FALSE;
    }
    return $id;
  }
}
