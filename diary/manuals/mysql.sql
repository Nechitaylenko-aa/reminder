CREATE DATABASE  IF NOT EXISTS `reminder` /*!40100 DEFAULT CHARACTER SET utf8mb3 COLLATE utf8mb3_bin */ /*!80016 DEFAULT ENCRYPTION='N' */;
USE `reminder`;
-- MySQL dump 10.13  Distrib 8.0.45, for Win64 (x86_64)
--
-- Host: 192.168.1.101    Database: reminder
-- ------------------------------------------------------
-- Server version	8.0.45

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
SET @MYSQLDUMP_TEMP_LOG_BIN = @@SESSION.SQL_LOG_BIN;
SET @@SESSION.SQL_LOG_BIN= 0;

--
-- GTID state at the beginning of the backup 
--

SET @@GLOBAL.GTID_PURGED=/*!80000 '+'*/ 'd9a2f4d3-2f13-11f1-84ac-202af993f0bf:1-2327';

--
-- Table structure for table `events`
--

DROP TABLE IF EXISTS `events`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `events` (
  `id` int NOT NULL AUTO_INCREMENT,
  `id_parent_i` int NOT NULL DEFAULT '0',
  `period_i` tinyint NOT NULL DEFAULT '5',
  `period_count_i` int NOT NULL DEFAULT '1',
  `event_ul` bigint NOT NULL DEFAULT '0',
  `was_shown_ul` bigint NOT NULL DEFAULT '0',
  `event_type_i` tinyint NOT NULL DEFAULT '0',
  `event_trigger_i` tinyint NOT NULL DEFAULT '0',
  `description_s` varchar(255) CHARACTER SET utf8mb3 COLLATE utf8mb3_bin NOT NULL,
  `priority_i` tinyint NOT NULL DEFAULT '0',
  `is_enabled_b` tinyint NOT NULL DEFAULT '1',
  `is_removed_b` tinyint NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `events`
--

LOCK TABLES `events` WRITE;
/*!40000 ALTER TABLE `events` DISABLE KEYS */;
INSERT INTO `events` VALUES (1,10,5,1,1779692400,0,0,1,'Аванс',1,1,0),(3,9,5,11,1778501436,1778501436,0,2,'Продувка системников',1,1,0);
/*!40000 ALTER TABLE `events` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `text_resources`
--

DROP TABLE IF EXISTS `text_resources`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `text_resources` (
  `id` int NOT NULL AUTO_INCREMENT,
  `id_parent_i` int NOT NULL,
  `descript_s` varchar(255) COLLATE utf8mb3_bin NOT NULL,
  `value_s` longtext COLLATE utf8mb3_bin NOT NULL,
  `type_i` tinyint NOT NULL DEFAULT '0',
  `is_removed_b` tinyint NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=35 DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `text_resources`
--

LOCK TABLES `text_resources` WRITE;
/*!40000 ALTER TABLE `text_resources` DISABLE KEYS */;
INSERT INTO `text_resources` VALUES (1,7,'Kyocera в главном зале','http://192.168.1.166',1,0),(2,8,'NAS samba','smb://192.168.1.208/software/',1,0),(30,8,'Sylve VE web-GUI','http://192.168.1.208:8182/',1,0),(31,8,'Sylve admin pass','admin',3,0),(32,8,'Nym projects','/home/artem/byn-projects',2,0),(34,8,'Config file','/etc/rc.conf',2,0);
/*!40000 ALTER TABLE `text_resources` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tracked_events_nodes`
--

DROP TABLE IF EXISTS `tracked_events_nodes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tracked_events_nodes` (
  `node_id` int NOT NULL,
  PRIMARY KEY (`node_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tracked_events_nodes`
--

LOCK TABLES `tracked_events_nodes` WRITE;
/*!40000 ALTER TABLE `tracked_events_nodes` DISABLE KEYS */;
INSERT INTO `tracked_events_nodes` VALUES (9),(10);
/*!40000 ALTER TABLE `tracked_events_nodes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tracked_text_nodes`
--

DROP TABLE IF EXISTS `tracked_text_nodes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tracked_text_nodes` (
  `node_id` int NOT NULL,
  PRIMARY KEY (`node_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tracked_text_nodes`
--

LOCK TABLES `tracked_text_nodes` WRITE;
/*!40000 ALTER TABLE `tracked_text_nodes` DISABLE KEYS */;
INSERT INTO `tracked_text_nodes` VALUES (7),(8);
/*!40000 ALTER TABLE `tracked_text_nodes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tree`
--

DROP TABLE IF EXISTS `tree`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tree` (
  `id` int NOT NULL AUTO_INCREMENT,
  `id_parent` int NOT NULL DEFAULT '0',
  `id_user_i` int NOT NULL DEFAULT '0',
  `res_type_i` tinyint NOT NULL DEFAULT '0',
  `title_s` varchar(255) COLLATE utf8mb3_bin NOT NULL DEFAULT '',
  `is_public_b` tinyint NOT NULL DEFAULT '0',
  `is_editable_b` tinyint NOT NULL DEFAULT '0',
  `is_container_b` tinyint NOT NULL DEFAULT '0',
  `is_admin_b` tinyint NOT NULL DEFAULT '0',
  `is_removed_b` tinyint NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `id_UNIQUE` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tree`
--

LOCK TABLES `tree` WRITE;
/*!40000 ALTER TABLE `tree` DISABLE KEYS */;
INSERT INTO `tree` VALUES (1,0,0,0,'Общее',1,0,0,1,0),(2,1,0,0,'Заметки',1,0,1,1,0),(3,1,0,1,'События',1,0,1,1,0),(4,0,0,0,'Пользовательские данные',0,0,0,1,0),(5,4,0,0,'Заметки',0,0,1,0,0),(6,4,0,1,'События',0,0,1,0,0),(7,2,1,0,'Сетевые принтеры',1,1,1,1,0),(8,2,1,0,'Сетевые шАры',1,1,1,1,0),(9,3,1,1,'Регламентные работы',1,1,1,1,0),(10,3,1,1,'Рутина',1,1,1,1,0);
/*!40000 ALTER TABLE `tree` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `users`
--

DROP TABLE IF EXISTS `users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `users` (
  `id` int NOT NULL AUTO_INCREMENT,
  `login_s` varchar(255) COLLATE utf8mb3_bin NOT NULL,
  `is_admin_i` tinyint NOT NULL DEFAULT '0',
  `passwd_s` varchar(45) CHARACTER SET utf8mb3 COLLATE utf8mb3_bin NOT NULL COMMENT 'fish - admin; 1234 - user',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `users`
--

LOCK TABLES `users` WRITE;
/*!40000 ALTER TABLE `users` DISABLE KEYS */;
INSERT INTO `users` VALUES (1,'admin',1,'83e4a96aed96436c621b9809e258b309'),(2,'user',0,'81dc9bdb52d04dc20036dbd8313ed055');
/*!40000 ALTER TABLE `users` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping routines for database 'reminder'
--
/*!50003 DROP PROCEDURE IF EXISTS `refresh_tracked_nodes` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`artem`@`192.168.1.%` PROCEDURE `refresh_tracked_nodes`()
BEGIN
    TRUNCATE TABLE tracked_text_nodes;
    TRUNCATE TABLE tracked_events_nodes;

    -- Для ноды 2
    INSERT INTO tracked_text_nodes (node_id)
    WITH RECURSIVE text_descendants AS (
        SELECT id FROM tree WHERE id = 2 AND is_removed_b = 0
        UNION ALL
        SELECT t.id
        FROM tree t
                 INNER JOIN text_descendants td ON t.id_parent = td.id
        WHERE t.is_removed_b = 0
    )
    SELECT id FROM text_descendants WHERE id != 2;

    -- Для ноды 3
    INSERT INTO tracked_events_nodes (node_id)
    WITH RECURSIVE events_descendants AS (
        SELECT id FROM tree WHERE id = 3 AND is_removed_b = 0
        UNION ALL
        SELECT t.id
        FROM tree t
                 INNER JOIN events_descendants ed ON t.id_parent = ed.id
        WHERE t.is_removed_b = 0
    )
    SELECT id FROM events_descendants WHERE id != 3;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
SET @@SESSION.SQL_LOG_BIN = @MYSQLDUMP_TEMP_LOG_BIN;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2026-05-12 17:02:35
