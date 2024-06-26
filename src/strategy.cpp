#include "strategy.h"

FutureAction::FutureAction(
    Vector2 target,
    int celerity,
    Radians orientation,
    bool activeKicker)
    : _target(Optional<MutableVector2>(target)),
      _celerity(celerity),
      _orientation(orientation),
      _activeKicker(activeKicker) {}

FutureAction::FutureAction(
    int celerity,
    Radians orientation,
    bool activeKicker)
    : _target(Optional<MutableVector2>()),
      _celerity(celerity),
      _orientation(orientation),
      _activeKicker(activeKicker) {}

////////
const int criticalWallDistance = 25;
const int goalMinDistance = 90; // 85 pour SN10 et 95 pour SN9 OUTDATED VALUES
const int myGoalMinDistance = 82; 
const int speedmotors = 120;
const FutureAction stopRobot = FutureAction(Vector2(0, 0), 0, 0, false);
FutureAction chooseStrategy(FieldProperties fP, RobotState cS, double orientation, Vector2 nearestWall) {
  if (robotIsLost(fP, cS)) {
    if (leavingField(fP, cS, nearestWall)) {
      return refrainFromLeavingStrategy(fP, cS, orientation, nearestWall);
    } else if (!ballIsDetected(fP, cS)) {
      SerialDebug.println("stopRobotStrategy");
      return stopRobot;

    } else if (ballIsCaught(fP, cS)) {
      if (!goalIsDetected(fP, cS)) {
        SerialDebug.println("stopRobotStrategy");
        return stopRobot;
      } else if (targetJustInFrontOfRobot(fP, cS, cS.enemyGoalPos())) {
        return shootStrategy(fP, cS);
      } else {
        return accelerateToGoalStrategyWithCam(fP, cS);
      }

    } else {
      if (targetInFrontOfRobotFromFront(fP, cS, cS.ballPos())) {
        return goToBallStrategy(fP, cS);
      } else {
        return goToBallAvoidingBallStrategyWithCam(fP, cS);
      }
    }

  } else {
    if (leavingField(fP, cS, nearestWall)) {
      return refrainFromLeavingStrategy(fP, cS, orientation, nearestWall);

    } else if (!ballIsDetected(fP, cS)) {
      return slalowingBackwardsStrategy(fP, cS);

    } else if (ballIsCaught(fP, cS)) {
      if (targetJustInFrontOfRobot(fP, cS, cS.myPos().distanceRef(fP.enemyGoalPos()))) {
        return shootStrategy(fP, cS);
      } else {
        return accelerateToGoalStrategyWithLidar(fP, cS);
      }

    } else {
      if (targetInFrontOfRobotFromFront(fP, cS, cS.ballPos())) {
        return goToBallStrategy(fP, cS);
      } else {
        return goToBallAvoidingBallStrategyWithLidar(fP, cS);
      }
    }
  }
}

bool robotIsLost(FieldProperties fP, RobotState cS) {
  return cS.myPos() == Vector2(-999.9, -999.9);
}

bool leavingField(FieldProperties fP, RobotState cS, Vector2 nearestWall) {
  if (!robotIsLost(fP, cS)) {
    SerialDebug.println("Left wall : " + String(cS.myPos().x() < -fP.fieldWidth() / 2 + 3*fP.robotRadius())
    + " Right wall : " + String((fP.fieldWidth() / 2) - 3*fP.robotRadius() < cS.myPos().x())
    + " Back wall : " + String(cS.myPos().y() < -fP.fieldLength() / 2 + 3*fP.robotRadius())
    + " Front wall : " + String(fP.fieldLength() / 2 - 3*fP.robotRadius() < cS.myPos().y())
    + " Enemy goal : " + String(cS.enemyGoalPos().norm() < goalMinDistance && cS.enemyGoalPos().norm() > 1)
    + " My goal : " + String(cS.myGoalPos().norm() < myGoalMinDistance && cS.myGoalPos().norm() > 1)
    + " Nearest wall : " + String(nearestWall.distance({0,0}) / 10.0 < criticalWallDistance));

    int distanceDevitementY;
    if (abs(cS.myPos().x()) < 40) {
      distanceDevitementY = 50;
    } else {
      distanceDevitementY = criticalWallDistance;
    }

    SerialDebug.println(distanceDevitementY);
    
    return (cS.myPos().x() < -fP.fieldWidth() / 2 + criticalWallDistance) ||
          (fP.fieldWidth() / 2 - criticalWallDistance < cS.myPos().x()) ||
          (cS.myPos().y() < -fP.fieldLength() / 2 + distanceDevitementY - 5) ||
          (fP.fieldLength() / 2 - distanceDevitementY < cS.myPos().y()) ||
          (cS.enemyGoalPos().norm() < goalMinDistance && cS.enemyGoalPos().norm() > 1) ||
          (cS.myGoalPos().norm() < myGoalMinDistance && cS.myGoalPos().norm() > 1) ||
          (nearestWall.distance({0,0}) / 10.0 < criticalWallDistance);
  } else {
    return (cS.enemyGoalPos().norm() < goalMinDistance && cS.enemyGoalPos().norm() > 1) ||
           (cS.myGoalPos().norm() < myGoalMinDistance && cS.myGoalPos().norm() > 1) ||
           (nearestWall.distance({0,0}) / 10.0 < criticalWallDistance);
  }
}

bool ballIsDetected(FieldProperties fP, RobotState cS) {
  return cS.ballPos() != Vector2(0, 0);
}

bool goalIsDetected(FieldProperties fP, RobotState cS) {
  return cS.enemyGoalPos() != Vector2(0, 0);
}

bool targetInFrontOfRobotFromFront(FieldProperties fP, RobotState cS, Vector2 tL) {
  float longRobot = (fP.robotRadius() * 1); // margin
  return tL.y() > longRobot;
}

bool targetInFrontOfRobotFromMiddle(FieldProperties fP, RobotState cS, Vector2 tL) {
  return tL.y() > 0;
}

bool targetCenterOfRobot(FieldProperties fP, RobotState cS, Vector2 tL) {
  return abs(tL.x()) <= 25;
}

bool targetJustInFrontOfRobot(FieldProperties fP, RobotState cS, Vector2 tL) {
  return targetInFrontOfRobotFromMiddle(fP, cS, tL) && targetCenterOfRobot(fP, cS, tL);
}

bool targetJustBehindOfRobot(FieldProperties fP, RobotState cS, Vector2 tL) {
  return (!targetInFrontOfRobotFromMiddle(fP, cS, tL)) && targetCenterOfRobot(fP, cS, tL);
}

bool ballIsCaught(FieldProperties fP, RobotState cS) {
  SerialDebug.println("ballIsCaught : " + String(targetJustInFrontOfRobot(fP, cS, cS.ballPos()) && cS.ballPos().y() <= 40));
  return targetJustInFrontOfRobot(fP, cS, cS.ballPos()) && cS.ballPos().y() <= 40;
}

FutureAction refrainFromLeavingStrategy(FieldProperties fP, RobotState cS, double orientation, Vector2 nearestWall) {
  SerialDebug.println("refrainFromLeavingStrategy");
  float xDirection = 0;
  float yDirection = 0;

  float orientationRadians = Radians(Degree(orientation));

  SerialDebug.println("orientationRadians: " + String(orientationRadians) + ", enemyGoalPos().norm: " + String(cS.enemyGoalPos().norm()));

  int distanceDevitementY;
  if (abs(cS.myPos().x()) < 40) {
    distanceDevitementY = 50;
  } else {
    distanceDevitementY = criticalWallDistance;
  }

  if (cS.enemyGoalPos().norm() < goalMinDistance && cS.enemyGoalPos().norm() > 1) {
    xDirection = -sin(orientationRadians);
    yDirection = -cos(orientationRadians);
  } else if (cS.myGoalPos().norm() < myGoalMinDistance && cS.myGoalPos().norm() > 1) {
    xDirection = sin(orientationRadians);
    yDirection = cos(orientationRadians);
  }

  if (!robotIsLost(fP, cS)) {
    if (cS.myPos().x() < -fP.fieldWidth() / 2 + criticalWallDistance) {
      xDirection = cos(orientationRadians);
      yDirection = sin(orientationRadians);
    } else if (fP.fieldWidth() / 2 - criticalWallDistance < cS.myPos().x()) {
      xDirection = -cos(orientationRadians);
      yDirection = -sin(orientationRadians);
    }

    if (cS.myPos().y() < -fP.fieldLength() / 2 + distanceDevitementY) {
      xDirection = sin(orientationRadians);
      yDirection = cos(orientationRadians);
    } else if (fP.fieldLength() / 2 - distanceDevitementY  - 5 < cS.myPos().y()) {
      xDirection = -sin(orientationRadians);
      yDirection = -cos(orientationRadians);
    }
  }

  if (nearestWall.distance({0,0}) / 10.0 < criticalWallDistance) {
    xDirection = -nearestWall.x();
    yDirection = -nearestWall.y();
  }


  SerialDebug.println(String(xDirection) + " " + String(yDirection));
  return FutureAction(
      Vector2(
          xDirection,
          yDirection),
      speedmotors,
      0,
      false);  //@Gandalfph add orientation and celerity
}

FutureAction goToBallStrategy(FieldProperties fP, RobotState cS) {
  SerialDebug.println("goToBallStrategy");
  

    int x_value;
  
  return FutureAction(
      Vector2(
          cS.ballPos().x(),
          cS.ballPos().y() - fP.robotRadius() * 4),
      speedmotors,
      0,
      false);  //@Gandalfph add orientation and celerity
}

FutureAction goToBallAvoidingBallStrategyWithCam(FieldProperties fP, RobotState cS) {
  SerialDebug.println("goToBallAvoidingBallStrategyWithCam");
  if (targetJustBehindOfRobot(fP, cS, cS.ballPos())) {
    return FutureAction(
        Vector2(10, -10),
        speedmotors,
        0,
        false);  //@Gandalfph add orientation and celerity

  } else if (cS.ballPos().x() < 0) {
    return FutureAction(
        Vector2(2, -10),
        speedmotors,
        0,
        false);  //@Gandalfph add orientation and celerity
  } else if (cS.ballPos().x() > 0) {
    return FutureAction(
        Vector2(-2, -10),
        speedmotors,
        0,
        false);  //@Gandalfph add orientation and celerity
  }
}

FutureAction goToBallAvoidingBallStrategyWithLidar(FieldProperties fP, RobotState cS) {
  SerialDebug.println("goToBallAvoidingBallStrategyWithLidar");
  if (targetJustBehindOfRobot(fP, cS, cS.ballPos())) {
    if (cS.ballPos().x() < 0) {
      return FutureAction(
          Vector2(10, -10),
          speedmotors,
          0,
          false);  //@Gandalfph add orientation and celerity

    } else {
      return FutureAction(
          Vector2(-10, -10),
          speedmotors,
          0,
          false);  //@Gandalfph add orientation and celerity
    }

  } else if (cS.ballPos().x() < 0 && cS.ballPos().x() > -40) {
    return FutureAction(
        Vector2(5, -10),
        speedmotors,
        0,
        false);  //@Gandalfph add orientation and celerity
  } else if (cS.ballPos().x() > 0 && cS.ballPos().x() < 40) {
    return FutureAction(
        Vector2(-5, -10),
        speedmotors,
        0,
        false);  //@Gandalfph add orientation and celerity
  } else {
    return FutureAction(
        Vector2(0, -10),
        speedmotors,
        0,
        false);
  }
}

FutureAction accelerateToGoalStrategyWithCam(FieldProperties fP, RobotState cS) {
  SerialDebug.println("accelerateToGoalStrategyWithCam");
  
  int offset;
  if (abs(cS.myGoalPos().x()) > 15) {
    offset = -cS.myPos().x()/10;
  }
  
  return FutureAction(
      Vector2(
          cS.enemyGoalPos().x(),
          cS.enemyGoalPos().y()),
      speedmotors,
      0,
      false);  //@Gandalfph add orientation and celerity
}

FutureAction accelerateToGoalStrategyWithLidar(FieldProperties fP, RobotState cS) {
  SerialDebug.println("accelerateToGoalStrategyWithLidar");
  
  int offset;
  if (abs(cS.myPos().x()) > 20) {
    offset = -cS.myPos().x()/10;
  }
  
  return FutureAction(
      Vector2(
          offset,
          10),
      speedmotors,
      0,
      false);  //@Gandalfph add orientation and celerity
}

FutureAction slalowingBackwardsStrategy(FieldProperties fP, RobotState cS) {
  SerialDebug.println("slalowingBackwardsStrategy");
  if (cS.myPos().y() < -50) {
    if (cS.myPos().x() < -5) {
      return FutureAction(
          Vector2(10, 0),
          speedmotors,
          0,
          false);  //@Gandalfph add orientation and celerity
    } else if (5 < cS.myPos().x()) {
      return FutureAction(
          Vector2(-10, 0),
          speedmotors,
          0,
          false);  //@Gandalfph add orientation and celerity
    } else {
      return FutureAction(
          Vector2(0, 10),
          speedmotors,
          0,
          false);  //@Gandalfph add orientation and celerity
    }

  } else if (50 < cS.myPos().y()) {
    return FutureAction(
        Vector2(-20, -10),
        speedmotors,
        0,
        false);  //@Gandalfph add orientation and celerity

  } else {
    if (cS.myPos().x() < -fP.fieldWidth() / 6) {
      return FutureAction(
          Vector2(20, -10),
          speedmotors,
          0,
          false);  //@Gandalfph add orientation and celerity
    } else if (fP.fieldWidth() / 6 < cS.myPos().x()) {
      return FutureAction(
          Vector2(-20, -10),
          speedmotors,
          0,
          false);  //@Gandalfph add orientation and celerity
    } else {
      return FutureAction(
          speedmotors,
          0,
          false);  //@Gandalfph add orientation and celerity
    }
  }
}

FutureAction shootStrategy(FieldProperties fP, RobotState cS) {
  SerialDebug.println("shootStrategy");
  
  // int shootSpeed = min(speedmotors*2, 255);

  int shootSpeed = 180;
  // if (speedmotors*2 > 255) {
  //   shootSpeed = 255;
  // } else {
  //   shootSpeed = speedmotors*2;
  // }
  
  return FutureAction(
      Vector2(0, 20),
      shootSpeed,
      0,
      true);  //@Gandalfph add orientation and celerity
}
