#!/bin/sh

APP_HOME=$(cd "$(dirname "$0")" && pwd)
APP_BASE_NAME=$(basename "$0")

DEFAULT_JVM_OPTS=""

CLASSPATH="$APP_HOME/gradle/wrapper/gradle-wrapper.jar:$APP_HOME/gradle/wrapper/gradle-wrapper-shared.jar"

if [ -n "$JAVA_HOME" ] ; then
    if [ -x "$JAVA_HOME/jre/sh/java" ] ; then
        JAVA_EXE="$JAVA_HOME/jre/sh/java"
    else
        JAVA_EXE="$JAVA_HOME/bin/java"
    fi
else
    JAVA_EXE="java"
fi

exec "$JAVA_EXE" $DEFAULT_JVM_OPTS $JAVA_OPTS $GRADLE_OPTS "-Dorg.gradle.appname=$APP_BASE_NAME" -classpath "$CLASSPATH" org.gradle.wrapper.GradleWrapperMain "$@"