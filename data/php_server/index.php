<?php
require_once __DIR__ . '/auth_store.php';

$sessionId = getSessionIdFromCookie();
$postData = getRequestPostData();
$requestMethod = $_SERVER['REQUEST_METHOD'] ?? getenv('REQUEST_METHOD') ?? 'GET';

if ($requestMethod === 'POST' && ($postData['action'] ?? '') === 'logout') {
	logoutSession($sessionId);
	header('Location: login.php');
	exit;
}

$currentUser = currentUserFromSession($sessionId);
?>
<!DOCTYPE html>
<html lang="pt-BR">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Area do Usuario</title>
	<style>
		* { margin: 0; padding: 0; box-sizing: border-box; }
		body { font-family: Arial, sans-serif; background: #f5f5f5; min-height: 100vh; display: flex; align-items: center; justify-content: center; }
		.card { background: white; width: 100%; max-width: 560px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); padding: 36px; }
		h1 { font-size: 26px; color: #222; margin-bottom: 12px; }
		p { color: #555; margin-bottom: 20px; line-height: 1.4; }
		.actions { display: flex; gap: 10px; flex-wrap: wrap; }
		.btn { display: inline-block; text-decoration: none; border: 0; border-radius: 4px; padding: 10px 14px; font-weight: bold; font-size: 14px; cursor: pointer; }
		.primary { background: #0d6efd; color: #fff; }
		.primary:hover { background: #0a58ca; }
		.danger { background: #dc3545; color: #fff; }
		.danger:hover { background: #bb2d3b; }
		.muted { background: #6c757d; color: #fff; }
		.muted:hover { background: #565e64; }
	</style>
</head>
<body>
	<div class="card">
		<?php if ($currentUser !== null): ?>
			<h1>Bem-vindo, <?php echo htmlspecialchars($currentUser); ?></h1>
			<p>Sua sessao esta ativa e vinculada ao cookie sessionId do webserv. A inatividade acima do limite remove a sessao automaticamente.</p>
			<div class="actions">
				<a class="btn primary" href="index.php">Atualizar pagina</a>
				<form method="POST" style="display:inline;">
					<input type="hidden" name="action" value="logout">
					<button type="submit" class="btn danger">Sair</button>
				</form>
			</div>
		<?php else: ?>
			<h1>Nenhum usuario logado</h1>
			<p>Faca login para vincular seu usuario ao sessionId fornecido pelo servidor.</p>
			<div class="actions">
				<a class="btn primary" href="login.php">Ir para login</a>
				<a class="btn muted" href="register.php">Criar conta</a>
			</div>
		<?php endif; ?>
	</div>
</body>
</html>
