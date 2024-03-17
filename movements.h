#ifndef MOUVEMENTS_H
#define MOUVEMENTS_H

#include <Arduino.h>
#include "utilities.h"

// TODO supprimer la mémorisation de sens en la remplaçant par une détection en temps réel grâce à FG
enum class Direction { forward,
                       backward,
                       stopped };

// TODO n'utilise pas FG (je ne sais pas comment faire), potentiellemnt remplacer _direction par une détection en directe grâce à FG
class MotorMov {
 public:
  MotorMov(
      uint8_t pinPWM,
      uint8_t pinCWCCW,
      uint8_t pinFG,
      float angleAxisKicker);
  void stop();
  void move(int value);
  // radians
  inline float angleAxisKicker() const { return _angleAxisKicker; }
  float anglePowerAxisKicker() const;

 private:
  const uint8_t _pinPWM;
  const uint8_t _pinCWCCW;
  const uint8_t _pinFG;
  const float _angleAxisKicker;  // radians

  Direction _direction;  // TODO remplacer par détection en direct via fg

  void _pwm(int value) const;
  void _cwccw(uint8_t value) const;
  uint8_t _fg() const;
};

class Motors {
 private:
  const MotorMov _frontRight;
  const MotorMov _frontLeft;
  const MotorMov _backRight;
  const MotorMov _backLeft;

 public:
  Motors(
      MotorMov frontRight,
      MotorMov frontLeft,
      MotorMov backRight,
      MotorMov backLeft);

  inline MotorMov frontRight() const { return _frontRight; }
  inline MotorMov frontLeft() const { return _frontLeft; }
  inline MotorMov backRight() const { return _backRight; }
  inline MotorMov backLeft() const { return _backLeft; }

  void fullStop();

  void goTo(Vector2 distances, int celerity);
};

#endif