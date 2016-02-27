MIGRATE
---------------------

The Migrate module provides a flexible framework for migrating content into Backdrop
from other sources (e.g., when converting a web site from another CMS to Backdrop).

Out-of-the-box, support for creating Backdrop nodes, taxonomy terms, comments, and
users are included. Plugins permit migration of other types of content.

CONTENTS OF THIS FILE
---------------------

 - Introduction
 - Tested
 - Known Issues
 - Special Thanks
 - Requirements
 - Installation
 - Coming From Drupal?
 - Usage
 - License
 - Credits
 - Maintainers

TESTED
-------

NOT WORKING.

This is an initial non-working port, which means much grunt work in converting CMI and plugins has been done but higher thought like "what/where is the registry table in Backdrop", "converting from non-MySQL databases to a MySQL database", "using Migrate with Drush in Backdrop", "do (generally enterprise) Oracle websystems want to migrate to Backdrop currently?", "migrating Poll module data and other removed modules in Backdrop -- what to do?" and other big topics have not been addressed yet.

This port is generally just preparing the way for a migration expert to finish it.  If you are that person, this module may be for you to address the big questions, finish and take ownership of.

KNOWN ISSUES
--------------

see above

A user migration with systemOfRecord == DESTINATION will drop pictures from user
records due to core bug http://drupal.org/node/935592 - the simpletests report an
error reflecting this. We have not developed a work-around.

Do not attempt to upgrade directly from Migrate 1 to Migrate 2! There is no
automated path to upgrade - your migrations (formerly known as "content sets")
must be reimplemented from scratch. It is recommended that projects using
Migrate 1 stay with Migrate 1, and that Migrate 2 be used for any new migration
projects.

SPECIAL THANKS
--------------

Much of the Migrate module functionality was sponsored by Cyrve, for its clients GenomeWeb
(http://www.genomeweb.com), The Economist (http://www.economist.com), and Examiner.com
(http://www.examiner.com).

Mike Ryan - http://drupal.org/user/4420
Moshe Weitzman - http://drupal.org/user/23

REQUIREMENTS
------------

professional developer experience

INSTALLATION
------------

Install this module using the official Backdrop CMS instructions at https://backdropcms.org/guide/modules

COMING FROM DRUPAL?
-------------------

This module relied heavily on importing classes, which have been turned into autoloading hooks.  The module does not break the site yet upon install but this converting has not been tested fully yet.

PERMISSIONS
------------

@todo


USAGE
------------

Documentation is at http://drupal.org/migrate. To get started, enable the
migrate_example module and browse to admin/content/migrate to see its dashboard.
The code for this migration is in migrate_example/beer.inc (advanced examples are
in wine.inc). Mimic that file in order to specify your own migrations.

The Migrate module itself has support for migration into core objects. Support
for migration involving contrib modules is in the migrate_extras module.

LICENSE
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for complete text.

CREDITS
-----------

Mike Ryan - http://drupal.org/user/4420
Moshe Weitzman - http://drupal.org/user/23

MAINTAINERS
-----------

 - seeking

Ported to Backdrop by:

 - biolithic <https://github.com/biolithic>
