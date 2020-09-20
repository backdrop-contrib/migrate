<?php
/**
 * @file
 * Our own hook implementation.
 */


/*
 * Implements hook_migrate_api().
 */
/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function migrate_migrate_api() {
  $api = array(
    'api' => MIGRATE_API_VERSION,
    'destination handlers' => array(
      'MigrateCommentNodeHandler',
      'MigrateFieldsEntityHandler',
      'MigratePathEntityHandler',
      'MigratePollEntityHandler',
      'MigrateStatisticsEntityHandler',
    ),
    'field handlers' => array(
      'MigrateTextFieldHandler',
      'MigrateValueFieldHandler',
      'MigrateNodeReferenceFieldHandler',
      'MigrateUserReferenceFieldHandler',
      'MigrateTaxonomyTermReferenceFieldHandler',
      'MigrateFileFieldHandler',
      'MigrateImageFieldHandler',
    ),
  );
  return $api;
}
