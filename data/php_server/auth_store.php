<?php

const USERS_DB_PATH = __DIR__ . '/users.db';
const SESSION_TIMEOUT_SECONDS = 600; // 10 minutes without activity

function getSessionIdFromCookie(): string
{
    $cookieHeader = $_SERVER['HTTP_COOKIE'] ?? getenv('HTTP_COOKIE') ?? '';

    foreach (explode(';', $cookieHeader) as $cookiePair) {
        $cookiePair = trim($cookiePair);
        if (strpos($cookiePair, 'sessionId=') === 0) {
            return substr($cookiePair, strlen('sessionId='));
        }
    }

    return '';
}

function getRequestPostData(): array
{
    if (($_SERVER['REQUEST_METHOD'] ?? '') !== 'POST') {
        return array();
    }

    if (!empty($_POST) && is_array($_POST)) {
        return $_POST;
    }

    $contentType = strtolower($_SERVER['CONTENT_TYPE'] ?? getenv('CONTENT_TYPE') ?? '');

    $raw = file_get_contents('php://input');
    if (!is_string($raw) || $raw === '') {
        $raw = file_get_contents('php://stdin');
    }

    if (!is_string($raw) || $raw === '') {
        return array();
    }

    if (strpos($contentType, 'application/x-www-form-urlencoded') !== false) {
        $data = array();
        parse_str($raw, $data);
        return is_array($data) ? $data : array();
    }

    if (strpos($contentType, 'application/json') !== false) {
        $data = json_decode($raw, true);
        return is_array($data) ? $data : array();
    }

    return array();
}

function withDbLock(callable $callback)
{
    $fp = fopen(USERS_DB_PATH, 'c+');
    if ($fp === false) {
        return null;
    }

    if (!flock($fp, LOCK_EX)) {
        fclose($fp);
        return null;
    }

    $size = filesize(USERS_DB_PATH);
    $raw = '';
    if ($size > 0) {
        $raw = fread($fp, $size);
    }

    $db = json_decode($raw, true);
    if (!is_array($db)) {
        $db = array();
    }
    if (!isset($db['users']) || !is_array($db['users'])) {
        $db['users'] = array();
    }
    if (!isset($db['sessions']) || !is_array($db['sessions'])) {
        $db['sessions'] = array();
    }

    $result = $callback($db);

    rewind($fp);
    ftruncate($fp, 0);
    fwrite($fp, json_encode($db, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE));
    fflush($fp);
    flock($fp, LOCK_UN);
    fclose($fp);

    return $result;
}

function cleanupExpiredSessions(array &$db): void
{
    $now = time();
    foreach ($db['sessions'] as $sid => $sessionData) {
        $lastSeen = $sessionData['last_seen'] ?? 0;
        if (!is_int($lastSeen)) {
            $lastSeen = (int) $lastSeen;
        }
        if (($now - $lastSeen) > SESSION_TIMEOUT_SECONDS) {
            unset($db['sessions'][$sid]);
        }
    }
}

function registerUser(string $username, string $password, string &$error): bool
{
    $username = trim($username);
    if ($username === '' || $password === '') {
        $error = 'Preencha usuario e senha.';
        return false;
    }

    if (!preg_match('/^[A-Za-z0-9_@\.-]{3,64}$/', $username)) {
        $error = 'Usuario deve ter 3-64 chars e usar letras, numeros, @, _, . ou -.';
        return false;
    }

    if (strlen($password) < 4) {
        $error = 'Senha muito curta.';
        return false;
    }

    $result = withDbLock(function (&$db) use ($username, $password, &$error) {
        cleanupExpiredSessions($db);

        if (isset($db['users'][$username])) {
            $error = 'Usuario ja existe.';
            return false;
        }

        $db['users'][$username] = array(
            'password_hash' => password_hash($password, PASSWORD_DEFAULT),
            'created_at' => time(),
        );
        return true;
    });

    if ($result === null) {
        $error = 'Falha ao abrir users.db.';
        return false;
    }

    return (bool) $result;
}

function loginUserWithSession(string $username, string $password, string $sessionId, string &$error): bool
{
    if ($sessionId === '') {
        $error = 'Sessao invalida. Recarregue a pagina.';
        return false;
    }

    $result = withDbLock(function (&$db) use ($username, $password, $sessionId, &$error) {
        cleanupExpiredSessions($db);

        if (!isset($db['users'][$username])) {
            $error = 'Usuario ou senha invalidos.';
            return false;
        }

        $hash = $db['users'][$username]['password_hash'] ?? '';
        if (!is_string($hash) || !password_verify($password, $hash)) {
            $error = 'Usuario ou senha invalidos.';
            return false;
        }

        $db['sessions'][$sessionId] = array(
            'username' => $username,
            'last_seen' => time(),
        );
        return true;
    });

    if ($result === null) {
        $error = 'Falha ao abrir users.db.';
        return false;
    }

    return (bool) $result;
}

function currentUserFromSession(string $sessionId): ?string
{
    if ($sessionId === '') {
        return null;
    }

    $result = withDbLock(function (&$db) use ($sessionId) {
        cleanupExpiredSessions($db);

        if (!isset($db['sessions'][$sessionId])) {
            return null;
        }

        $username = $db['sessions'][$sessionId]['username'] ?? null;
        if (!is_string($username) || $username === '') {
            unset($db['sessions'][$sessionId]);
            return null;
        }

        $db['sessions'][$sessionId]['last_seen'] = time();
        return $username;
    });

    if ($result === null || $result === '') {
        return null;
    }

    return $result;
}

function logoutSession(string $sessionId): void
{
    if ($sessionId === '') {
        return;
    }

    withDbLock(function (&$db) use ($sessionId) {
        cleanupExpiredSessions($db);
        unset($db['sessions'][$sessionId]);
        return true;
    });
}
