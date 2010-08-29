<?php
// $Id$

/**
 * @file
 * Advanced migration examples. These serve two purposes:
 *
 * 1. To demonstrate some of the more advanced usages of the Migrate module.
 *    Search for "TIP:" below for features not found in the basic example.
 * 2. To provide thorough test cases for the simpletest suite.
 *
 */

/**
 * Abstract intermediate class holding common settings.
 */
abstract class AdvancedExampleMigration extends Migration {
  public $basicFormat;

  public function __construct() {
    parent::__construct();
    $this->team = array(
      new MigrateTeamMember('Jack Kramer', 'jkramer@example.com', t('Taster')),
      new MigrateTeamMember('Linda Madison', 'lmadison@example.com', t('Winemaker')),
    );
    $this->issuePattern = 'http://drupal.org/node/:id:';

    // A format of our own, for testing migration of formats
    $formats = filter_formats();
    foreach ($formats as $format) {
      if ($format->name == 'Migrate example format') {
        $this->basicFormat = $format->format;
        break;
      }
    }
  }
}

/**
 * TIP: While usually you'll create true migrations - processes that copy data
 * from some source into Drupal - you can also define processing steps for either
 * the import or rollback stages that take other actions. In this case, we want
 * to disable pathauto while the migration steps run.
 */
class WinePrepMigration extends MigrationBase {
  public function __construct() {
    parent::__construct();
    $this->description = t('If pathauto is present, disable it for the duration');
    // TIP: Regular dependencies, besides enforcing (in the absence of --force)
    // the run order of migrations, affect the sorting of migrations on display.
    // You can use soft dependencies to affect just the display order when the
    // migrations aren't technically required to run in a certain order. In this
    // case, we want the wine migrations to appear after the beer migrations -
    // without this line, they would be intermingled due to their lack of
    // (formal) interdependencies.
    $this->softDependencies = array('BeerCommentMigration');
  }
  // Define isComplete(), returning a boolean, to indicate whether dependent
  // migrations may proceed
  public function isComplete() {
    // If pathauto is disabled, other migrations are free to go
    if (module_exists('pathauto')) {
      return FALSE;
    }
    else {
      return TRUE;
    }
  }
  // Implement any action you want to occur during an import process in an
  // import() process (alternatively, if you have an action which you want to
  // run during rollbacks, define a rollback() method).
  public function import() {
    if (module_exists('pathauto')) {
      module_disable(array('pathauto'));
      $this->showMessage(t('Disabled pathauto module'), 'success');
    }
    else {
      $this->showMessage(t('Pathauto is already disabled'), 'success');
    }
    // Must return one of the MigrationBase RESULT constants
    return MigrationBase::RESULT_COMPLETED;
  }
}

// The term migrations are very similar - implement the commonalities here
abstract class WineTermMigration extends AdvancedExampleMigration {
  public function __construct($type, $vocabulary_name, $description) {
    parent::__construct();
    $this->description = $description;
    $this->dependencies = array('WinePrepMigration');
    $this->map = new MigrateSQLMap($this->machineName,
        array(
          'categoryid' => array('type' => 'int',
                                'unsigned' => TRUE,
                                'not null' => TRUE,
                               )
        ),
        MigrateDestinationTerm::getKeySchema()
      );

    $query = db_select('migrate_example_wine_categories', 'wc')
             ->fields('wc', array('categoryid', 'name', 'details', 'category_parent', 'ordering'))
             ->condition('type', $type)
             // This sort assures that parents are saved before children.
             ->orderBy('category_parent', 'ASC');
    $this->source = new MigrateSourceSQL($this, $query);
    $this->destination = new MigrateDestinationTerm($vocabulary_name);

    // Mapped fields
    $this->addFieldMapping('name', 'name');
    $this->addFieldMapping('description', 'details');
    $this->addFieldMapping('parent', 'category_parent')
         ->sourceMigration(get_class($this));
    $this->addFieldMapping('weight', 'ordering');
    $this->addFieldMapping('format')
         ->defaultValue($this->basicFormat);

    // Unmapped source fields

    // Unmapped destination fields
    $this->addFieldMapping('parent_name')
         ->issueGroup(t('DNM'));
  }
}

class WineVarietyMigration extends WineTermMigration {
  public function __construct() {
    parent::__construct('variety', 'migrate_example_wine_varieties',
      t('Migrate varieties from the source database to taxonomy terms'));
  }
}

class WineRegionMigration extends WineTermMigration {
  public function __construct() {
    parent::__construct('region', 'migrate_example_wine_regions',
      t('Migrate regions from the source database to taxonomy terms'));
  }
}

class WineBestWithMigration extends WineTermMigration {
  public function __construct() {
    parent::__construct('best_with', 'migrate_example_wine_best_with',
      t('Migrate "Best With" from the source database to taxonomy terms'));
  }
}

class WineUserMigration extends AdvancedExampleMigration {
  public function __construct() {
    parent::__construct();
    $this->description = t('Wine Drinkers of the world');
    $this->dependencies = array('WinePrepMigration');
    $this->map = new MigrateSQLMap($this->machineName,
        array('accountid' => array(
                'type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Account ID.'
                )
             ),
        MigrateDestinationUser::getKeySchema()
    );
    $query = db_select('migrate_example_wine_account', 'wa')
             ->fields('wa', array('accountid', 'status', 'posted', 'name',
                'password', 'mail', 'last_access', 'last_login',
                'original_mail', 'sig', 'sex'));
    $this->source = new MigrateSourceSQL($this, $query);
    $this->destination = new MigrateDestinationUser();

    // Mapped fields
    $this->addFieldMapping('name', 'name');
    $this->addFieldMapping('status', 'status');
    $this->addFieldMapping('created', 'posted')
         ->description('See prepare method');
    $this->addFieldMapping('access', 'last_access')
         ->description('See prepare method');
    $this->addFieldMapping('login', 'last_login')
         ->description('See prepare method');
    $this->addFieldMapping('mail', 'mail');
    $this->addFieldMapping('pass', 'password');
    $this->addFieldMapping('roles')
         ->defaultValue(drupal_map_assoc(array(2)));
    $this->addFieldMapping('signature', 'sig');
    $this->addFieldMapping('signature_format')
         ->defaultValue($this->basicFormat);
    $this->addFieldMapping('init', 'original_mail');
    $this->addFieldMapping('field_migrate_example_gender', 'sex')
         ->description(t('Map from M/F to 0/1 in prepare method'));

    // Unmapped source fields

    // Unmapped destination fields
    $this->addFieldMapping('theme')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping('timezone')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping('language')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping('picture')
         ->issueGroup(t('DNM'));
  }

  public function prepare(stdClass $account, stdClass $row) {
    // Source dates are in ISO format.
    // Because the mappings above have been applied, $account->created contains
    // the date/time string now - we could also pass $row->posted here.
    $account->created = strtotime($account->created);
    $account->access = strtotime($account->access);
    $account->login = strtotime($account->login);

    // Gender data comes in as M/F, needs to be saved as Male=0/Female=1
    // TIP: Note that the Migration prepare method is called after all other
    // prepare handlers. Most notably, the field handlers have had their way
    // and created field arrays, so we have to save in the same format.
    switch ($row->sex) {
      case 'm':
      case 'M':
        $account->field_migrate_example_gender[LANGUAGE_NONE][0]['value'] = 0;
        break;
      case 'f':
      case 'F':
        $account->field_migrate_example_gender[LANGUAGE_NONE][0]['value'] = 1;
        break;
      default:
        unset($account->field_migrate_example_gender);
        break;
    }
  }
}

class WineProducerMigration extends AdvancedExampleMigration {
  public function __construct() {
    parent::__construct();
    $this->description = t('Wine producers of the world');
    $this->dependencies = array('WineRegionMigration', 'WineUserMigration');

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'producerid' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'alias' => 'p',
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $query = db_select('migrate_example_wine_producer', 'p')
             ->fields('p', array('producerid', 'name', 'body', 'excerpt', 'accountid'));
    // Region term is singletons, handled straighforwardly
    $query->leftJoin('migrate_example_wine_category_producer', 'reg',
      "p.producerid = reg.producerid");
    $query->addField('reg', 'categoryid', 'region');

    $this->source = new MigrateSourceSQL($this, $query);
    $this->destination = new MigrateDestinationNode('migrate_example_producer');

    // Mapped fields
    $this->addFieldMapping('title', 'name')
         ->description(t('Mapping producer name in source to node title'));
    $this->addFieldMapping('uid', 'accountid')
         ->sourceMigration('WineUserMigration')
         ->defaultValue(1);
    $this->addFieldMapping('migrate_example_wine_regions', 'region')
         ->sourceMigration('WineRegionMigration')
         ->arguments(array('source_type' => 'tid'));
    $arguments = MigrateTextFieldHandler::arguments(array('source_field' => 'excerpt'));
    $this->addFieldMapping('body', 'body')
         ->arguments($arguments);
    $this->addFieldMapping(NULL, 'excerpt');
    $this->addFieldMapping('sticky')
         ->defaultValue(0);

    // No unmapped source fields

    // Unmapped destination fields
    $this->addFieldMapping('is_new')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping('name')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping('created')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping('changed')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping('status')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping('promote')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping('revision')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping('language')
         ->issueGroup(t('DNM'));
  }
}

/**
 * TIP: An example of importing from an XML feed. See the files in the xml
 * directory - index.xml contains a list of IDs to import, and <id>.xml
 * is the data for a given producer.
 *
 * First, you need to extend MigrateXMLSource, implementing count(),
 * populateIdlist(), and constructObjectUrl().
 */
class WineXMLSource extends MigrateXMLSource {
  /**
   * Return a count of all available source records.
   */
  public function count($refresh = FALSE) {
    $xml = simplexml_load_file($this->listUrl);
    // Number of sourceid elements beneath the top-level element
    $count = count($xml->sourceid);
    return $count;
  }

  /**
   * Return an array of IDs to be imported.
   *
   * @param $xml
   *  SimpleXMLElement object representing the listing file.
   */
  protected function populateIdlist($xml) {
    $this->idList = (array)$xml->sourceid;
  }

  /**
   * Given the ID of an object to import, construct the URL to retrieve it
   *
   * @param $id
   */
  protected function constructObjectUrl($id) {
    $object_url = 'http://127.0.0.1/' . drupal_get_path('module', 'migrate_example') .
      "/xml/$id.xml";
    return $object_url;
  }
}

/**
 * Note that, if basing a migration on an XML source, you need to derive it
 * from XMLMigration instead of Migration.
 */
class WineProducerXMLMigration extends XMLMigration {
  public function __construct() {
    parent::__construct();
    $this->description = t('XML feed of wine producers of the world');
    $this->dependencies = array('WineRegionMigration', 'WineUserMigration');

    // The source ID here is the one retrieved from the XML listing file, and
    // used to identify the specific object's file
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'sourceid' => array(
          'type' => 'varchar',
          'length' => 4,
          'not null' => TRUE,
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );

    // The key argument for XML source object is the URL of the listing
    $this->source = new WineXMLSource($this,
      'http://127.0.0.1/' . drupal_get_path('module', 'migrate_example') . '/xml/index.xml');

    $this->destination = new MigrateDestinationNode('migrate_example_producer');

    // Note that for XML sources, in addition to the source field passed to
    // addFieldMapping (the name under which it will be saved in the data row
    // passed through the migration process) we specify the Xpath used to retrieve
    // the value from the XML.
    $this->addFieldMapping('title', 'title')
         ->xpath('/producer/name');
    $this->addFieldMapping('uid', 'authorid')
         ->xpath('/producer/authorid')
         ->sourceMigration('WineUserMigration')
         ->defaultValue(1);
    $this->addFieldMapping('migrate_example_wine_regions', 'region')
         ->xpath('/producer/region');
    $this->addFieldMapping('body', 'description')
         ->xpath('/producer/description');
  }
}

// TODO: Add node_reference field pointing to producer
class WineWineMigration extends AdvancedExampleMigration {
  public function __construct() {
    parent::__construct();
    $this->description = t('Wines of the world');
    $this->dependencies = array('WineVarietyMigration', 'WineRegionMigration',
      'WineBestWithMigration', 'WineUserMigration', 'WineProducerMigration');

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'wineid' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'Wine ID',
          'alias' => 'w',
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $query = db_select('migrate_example_wine', 'w')
             ->fields('w', array('wineid', 'name', 'body', 'excerpt', 'accountid',
              'image', 'posted', 'last_changed', 'variety', 'region', 'rating'));
    $query->leftJoin('migrate_example_wine_category_wine', 'cwbw',
      "w.wineid = cwbw.wineid");
    $query->leftJoin('migrate_example_wine_categories', 'bw',
      "cwbw.categoryid = bw.categoryid AND bw.type = 'best_with'");
    // Gives a single comma-separated list of related terms
    $query->groupBy('cwbw.wineid');
    $query->addExpression('GROUP_CONCAT(bw.categoryid)', 'best_with');

    $count_query = db_select('migrate_example_wine', 'w');
    $count_query->addExpression('COUNT(wineid)', 'cnt');

    $this->source = new MigrateSourceSQL($this, $query, $count_query);
    $this->destination = new MigrateDestinationNode('migrate_example_wine');

    // Mapped fields
    $this->addFieldMapping('title', 'name')
         ->description(t('Mapping wine name in source to node title'));
    $this->addFieldMapping('uid', 'accountid')
         ->sourceMigration('WineUserMigration')
         ->defaultValue(1);
    // TIP: By default, term relationship are assumed to be passed by name.
    // In this case, the source values are IDs, so we specify the relevant
    // migration (so the tid can be looked up in the map), and tell the term
    // field handler that it is receiving tids instead of names
    $this->addFieldMapping('migrate_example_wine_varieties', 'variety')
         ->sourceMigration('WineVarietyMigration')
         ->arguments(array('source_type' => 'tid'));
    $this->addFieldMapping('migrate_example_wine_regions', 'region')
         ->sourceMigration('WineRegionMigration')
         ->arguments(array('source_type' => 'tid'));
    $this->addFieldMapping('migrate_example_wine_best_with', 'best_with')
         ->separator(',')
         ->sourceMigration('WineBestWithMigration')
         ->arguments(array('source_type' => 'tid'));
    $this->addFieldMapping('field_migrate_example_wine_ratin', 'rating');
    $arguments = MigrateTextFieldHandler::arguments(array('source_field' => 'excerpt'));
    $this->addFieldMapping('body', 'body')
         ->arguments($arguments);
    $this->addFieldMapping(NULL, 'excerpt');
    $arguments = MigrateFileFieldHandler::arguments(drupal_get_path('module', 'migrate_example'), 'file_copy', FILE_EXISTS_RENAME);
    $this->addFieldMapping('field_migrate_example_image', 'image')
         ->arguments($arguments);
    $this->addFieldMapping('sticky')
         ->defaultValue(0);
    // These are already UNIX timestamps, so just pass through
    $this->addFieldMapping('created', 'posted');
    $this->addFieldMapping('changed', 'last_changed');

    // No unmapped source fields

    // Unmapped destination fields
    $this->addFieldMapping('is_new')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping('status')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping('promote')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping('revision')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping('language')
         ->issueGroup(t('DNM'));
  }
}

class WineCommentMigration extends AdvancedExampleMigration {
  public function __construct() {
    parent::__construct();
    $this->description = 'Comments about wines';
    $this->dependencies = array('WineUserMigration', 'WineWineMigration');
    $this->map = new MigrateSQLMap($this->machineName,
      array('commentid' => array(
              'type' => 'int',
              'unsigned' => TRUE,
              'not null' => TRUE,
             )
           ),
        MigrateDestinationComment::getKeySchema()
      );
    $query = db_select('migrate_example_wine_comment', 'wc')
             ->fields('wc', array('commentid', 'comment_parent', 'name', 'mail',
              'accountid', 'body', 'wineid', 'subject', 'commenthost', 'userpage',
              'posted', 'lastchanged'))
             ->orderBy('comment_parent');
    $this->source = new MigrateSourceSQL($this, $query);
    $this->destination = new MigrateDestinationComment('comment_node_migrate_example_wine');

    // Mapped fields
    $this->addFieldMapping('name', 'name');
    $this->addFieldMapping('subject', 'subject');
    $this->addFieldMapping('mail', 'mail');
    $this->addFieldMapping('status')
         ->defaultValue(COMMENT_PUBLISHED);
    $this->addFieldMapping('nid', 'wineid')
         ->sourceMigration('WineWineMigration');
    $this->addFieldMapping('uid', 'accountid')
         ->sourceMigration('WineUserMigration')
         ->defaultValue(0);
    $this->addFieldMapping('pid', 'comment_parent')
         ->sourceMigration('WineCommentMigration')
         ->description('Parent comment');
    $this->addFieldMapping('comment_body', 'body');
    $this->addFieldMapping('hostname', 'commenthost');
    $this->addFieldMapping('created', 'posted');
    $this->addFieldMapping('changed', 'lastchanged');
    $this->addFieldMapping('homepage', 'userpage');

    // No unmapped source fields

    // Unmapped destination fields
    $this->addFieldMapping('thread')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping('language')
         ->issueGroup(t('DNM'));
  }
}

class WineFinishMigration extends MigrationBase {
  public function __construct() {
    parent::__construct();
    $this->description = t('If pathauto is present and was previously enabled,
      re-enable it');
    $this->dependencies = array('WineCommentMigration');
  }
  public function isComplete() {
    if (module_exists('pathauto')) {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }
  public function import() {
    if (!module_exists('pathauto')) {
      // TODO: Really should keep track of whether it was originally enabled
      module_enable(array('pathauto'));
      $this->showMessage(t('Re-enabled pathauto module'), 'success');
    }
    else {
      $this->showMessage(t('Pathauto module already enabled', 'success'));
    }
    return Migration::RESULT_COMPLETED;
  }
}