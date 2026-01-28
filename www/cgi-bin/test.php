<?php
echo "<h1>Ola do PHP!</h1>";
echo "<p>Se voce esta vendo isso, o CGI funciona.</p>";

echo "<h2>Variaveis de Ambiente Recebidas:</h2>";
echo "<ul>";
foreach (getenv() as $key => $value) {
    echo "<li><strong>$key</strong>: $value</li>";
}
echo "</ul>";
?>