MIGRATE
-------

The Migrate module provides a flexible framework for migrating content into
Backdrop CMS from other sources (for example, when converting a web site from
another CMS to Backdrop).

Out-of-the-box, support for creating Backdrop CMS nodes, taxonomy terms,
comments, and users are included. Plugins permit migration of other types of
content.

*The port of the mofule from Drupal 7 to Backdrop CMS is still in process, and
not everything has been tested.*


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
------

While basic functionality might be working, higher thought like "what/where is
the registry table in Backdrop", "converting from non-MySQL databases to a MySQL
database", "using Migrate with Drush in Backdrop", "do (generally enterprise)
Oracle websystems want to migrate to Backdrop currently?", "what to do when
migrating Poll module data and data from other modules removed from Backdrop?"
and other big topics have not been addressed yet.


KNOWN ISSUES
------------

See above.

A user migration with systemOfRecord == DESTINATION will drop pictures from user
records due to core bug http://drupal.org/node/935592 - the SimpleTests report
an error reflecting this. We have not developed a work-around.

Do not attempt to upgrade directly from Migrate 1 to Migrate 2! There is no
automated path to upgrade. Your migrations (formerly known as "content sets")
must be re-implemented from scratch. It is recommended that projects using
Migrate 1 stay with Migrate 1, and that Migrate 2 be used for any new migration
projects.


SPECIAL THANKS
--------------

Much of the Migrate module functionality was sponsored by Cyrve, for its clients
[GenomeWeb](http://www.genomeweb.com),
[The Economist](http://www.economist.com), and
[Examiner.com](http://www.examiner.com).

[Mike Ryan](http://drupal.org/user/4420)
[Moshe Weitzman](http://drupal.org/user/23)


REQUIREMENTS
------------

None.


INSTALLATION
------------

Install this module using the official Backdrop CMS instructions at
https://backdropcms.org/guide/modules


COMING FROM DRUPAL?
-------------------

This module relied heavily on importing classes, which have been turned into
autoloading hooks. The module does not break the site upon install but this
converting has not been fully tested yet.


PERMISSIONS
-----------

@todo


USAGE
-----

Documentation is currently at http://drupal.org/migrate, but will eventually be
copied to https://github.com/backdrop-contrib/migrate/wiki.

To get started, enable the migrate_example module and navigate in your web
browser to admin/content/migrate to see its dashboard. The code for that
migration is in migrate_example/beer.inc (advanced examples are in wine.inc).
Mimic that file in order to specify your own migrations.

The Migrate module itself has support for migration into core objects. Support
for migration involving contrib modules is in the migrate_extras module or in
the module the specific data is for.


LICENSE
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.


CREDITS
-----------

- [Mike Ryan](http://drupal.org/user/4420)
- [Moshe Weitzman](http://drupal.org/user/23)


MAINTAINERS
-----------

- [Jason Flatt](https://github.com/oadaeh)

Original port to Backdrop CMS started by:

- [biolithic](https://github.com/biolithic)
