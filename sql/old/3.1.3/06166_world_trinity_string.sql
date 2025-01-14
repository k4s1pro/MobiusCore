INSERT INTO `server_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES
    (756, 'Battle begins!', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
    (757, '%s has successfully defended the fortress!', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
    (758, '%s has taken over the fortress!', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
    (759, 'The %s siege workshop has been damaged by the %s!', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
    (760, 'The %s siege workshop has been destroyed by the %s!', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
    (761, 'The %s tower has been damaged!', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
    (762, 'The %s tower has been destroyed!', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
    (763, 'Wintergrasp fortress is under attack!', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
    (764, 'Wintergrasp is now under the control of the %s.', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
    (765, 'Wintergrasp timer set to %s.', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
    (766, 'Wintergrasp battle started.', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
    (767, 'Wintergrasp battle finished.', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
    (768, 'Wintergrasp info: %s controled. Timer: %s. Wartime: %s', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
    (769, 'Wintergrasp outdoorPvP is disabled.', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
    (770, 'Wintergrasp outdoorPvP is enabled.', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
    (771, 'You have reached Rank 1: Corporal', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
    (772, 'You have reached Rank 2: First Lieutenant', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
INSERT INTO command (name, security, help) VALUES
    ('wg', 3, 'Syntax: .wg $subcommand.'),
    ('wg enable', 3, 'Syntax: .wg enable [on/off] Enable/Disable Wintergrasp outdoorPvP.'),
    ('wg start', 3, 'Syntax: .wg start\r\nForce Wintergrasp battle start.'),
    ('wg status', 3, 'Syntax: .wg status\r\nWintergrasp info, defender, timer, wartime.'),
    ('wg stop', 3, 'Syntax: .wg stop\r\nForce Wintergrasp battle stop (No rewards).'),
    ('wg switch', 3, 'Syntax: .wg switch\r\nSwitchs Wintergrasp defender team.'),
    ('wg timer', 3, 'Syntax: .wg timer\r\nChange Wintergrasp current timer (in secs).');
