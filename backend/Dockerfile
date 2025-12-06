FROM eclipse-temurin:17-jdk-alpine as build

WORKDIR /build

# Copiar archivos de Maven
COPY pom.xml .
COPY src/ src/

# Instalar Maven
RUN apk add --no-cache maven

# Compilar la aplicación
RUN mvn clean package -DskipTests

FROM eclipse-temurin:17-jre-alpine

WORKDIR /app

# Copiar JAR del stage anterior
COPY --from=build /build/target/*.jar app.jar

# Puerto expuesto
EXPOSE 20001

# Ejecutar la aplicación
ENTRYPOINT ["java", "-jar", "app.jar"]
