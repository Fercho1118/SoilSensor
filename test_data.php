<?php
// Definición de parámetros para la conexión con la base de datos MySQL.
$hostname = "localhost";
$username = "root";
$password = "";
$database = "sensor_db";

// Intento de conexión con la base de datos.
$conn = mysqli_connect($hostname, $username, $password, $database);

// Verifica si la conexión fue exitosa. En caso contrario, termina el script.
if (!$conn) {
    die("Conexión fallida: " . mysqli_connect_error()) ;
}

echo "Conexión a la base de datos correcta. ";

// Verifica si se recibieron los datos de temperatura y humedad por el método POST.
if (isset($_POST["temperature"]) && isset($_POST["humidity"])) {
    // Convierte los valores recibidos a flotante para asegurar el tipo de dato correcto.
    $t = floatval($_POST["temperature"]);
    $h = floatval($_POST["humidity"]);

    // Preparación de la consulta SQL para evitar inyecciones SQL.
    $stmt = $conn->prepare("INSERT INTO seesaw (temperature, humidity) VALUES (?, ?)");

    // Vincula los parámetros a la consulta SQL.
    $stmt->bind_param("dd", $t, $h);

    // Ejecuta la consulta y verifica si fue exitosa.
    if ($stmt->execute()) {
        echo "Nuevo dato agregado correctamente";
    } else {
        // En caso de error, muestra el mensaje de error.
        echo "Error: " . $stmt->error;
    }
    // Cierra el statement.
    $stmt->close();
}
// Cierra la conexión a la base de datos.
mysqli_close($conn);
?>