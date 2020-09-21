<?php

/**
 * @file
 * Custom exception class for the migrate module.
 */

/**
 * Defines the MigrateException class.
 */
class MigrateException extends Exception {

  /**
   * The level of the error being reported (a Migration::MESSAGE_* constant)
   *
   * @var int
   */
  protected $level;

  public function getLevel() {
    return $this->level;
  }

  /**
   * The status to record in the map table for the current item (a
   * MigrateMap::STATUS_* constant)
   *
   * @var int
   */
  protected $status;

  public function getStatus() {
    return $this->status;
  }

  /**
   * Creates the MigrateException object.
   *
   * @param string $message
   *   The message for the error.
   * @param int $level
   *   The level of the error being reported.
   * @param int $status
   *   The status for the current item.
   */
  public function __construct($message, $level = Migration::MESSAGE_ERROR, $status = MigrateMap::STATUS_FAILED) {
    $this->level = $level;
    $this->status = $status;
    parent::__construct($message);
  }
}
