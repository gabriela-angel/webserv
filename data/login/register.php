<?php
require_once __DIR__ . '/auth_store.php';

$error = '';
$success = '';
$sessionId = getSessionIdFromCookie();
$currentUser = currentUserFromSession($sessionId);
$requestMethod = $_SERVER['REQUEST_METHOD'] ?? getenv('REQUEST_METHOD') ?? 'GET';

if ($currentUser !== null) {
	header('Location: index.php');
	exit;
}

if ($requestMethod === 'POST') {
	$postData = getRequestPostData();
	$username = trim($postData['username'] ?? '');
	$password = $postData['password'] ?? '';
	$confirmPassword = $postData['confirm_password'] ?? '';

	if ($password !== $confirmPassword) {
		$error = 'As senhas nao coincidem.';
	} else {
		if (registerUser($username, $password, $error)) {
			$success = 'Usuario criado com sucesso. Faca login.';
		}
	}
}
?>
<!DOCTYPE html>
<html lang="pt-BR">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Registro</title>
	<style>
		* { margin: 0; padding: 0; box-sizing: border-box; }
		body { font-family: Arial, sans-serif; background: #f5f5f5; display: flex; justify-content: center; align-items: center; height: 100vh; }
		.card { background: white; padding: 40px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); width: 100%; max-width: 420px; }
		h1 { text-align: center; margin-bottom: 30px; color: #333; }
		.form-group { margin-bottom: 16px; }
		label { display: block; margin-bottom: 5px; color: #555; font-weight: bold; }
		input { width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 4px; font-size: 14px; }
		input:focus { outline: none; border-color: #007bff; }
		button { width: 100%; padding: 12px; background: #198754; color: white; border: none; border-radius: 4px; font-size: 16px; cursor: pointer; font-weight: bold; }
		button:hover { background: #146c43; }
		.error { color: #dc3545; text-align: center; margin-bottom: 16px; }
		.success { color: #198754; text-align: center; margin-bottom: 16px; }
		a { display: block; text-align: center; margin-top: 16px; color: #007bff; text-decoration: none; }
		a:hover { text-decoration: underline; }
	</style>
</head>
<body>
	<div class="card">
		<h1>Registrar</h1>
		<?php if ($error): ?>
			<div class="error"><?php echo htmlspecialchars($error); ?></div>
		<?php endif; ?>
		<?php if ($success): ?>
			<div class="success"><?php echo htmlspecialchars($success); ?></div>
		<?php endif; ?>
		<form method="POST">
			<div class="form-group">
				<label for="username">Usuario</label>
				<input type="text" id="username" name="username" required>
			</div>
			<div class="form-group">
				<label for="password">Senha</label>
				<input type="password" id="password" name="password" required>
			</div>
			<div class="form-group">
				<label for="confirm_password">Confirmar senha</label>
				<input type="password" id="confirm_password" name="confirm_password" required>
			</div>
			<button type="submit">Criar conta</button>
		</form>
		<a href="login.php">Voltar para login</a>
	</div>
</body>
</html>
