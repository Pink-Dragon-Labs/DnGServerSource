/*
 * Deletes unused columns for foes so that we can /pignore again.
 */

ALTER TABLE realmdb.foes
  DROP COLUMN name,
  DROP COLUMN FriendNameID;