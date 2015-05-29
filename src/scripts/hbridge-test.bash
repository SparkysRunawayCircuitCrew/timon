#!/bin/bash
#
#  Simple script to check H-Bridge speed controller used on timon.
#
#  - Exercises both motor controllers
#  - Test all 4 combinations of digital outputs
#  - For each combination, tries 50%, 100%, 0% duty cycle

declare gpioDir="/sys/class/gpio";
declare pwmDir="/sys/devices/ocp.3";
declare SLOTS="/sys/devices/bone_capemgr.9/slots";

# PWM waveform period (in nanoseconds)
declare -i period=1000000;

disable_gpio() {
  declare gpio="${1}";
  declare gpioEntry="${gpioDir}/gpio${gpio}";

  if [ -e "${gpioEntry}" ]; then
    echo ${gpio} > "${gpioDir}/unexport";
  fi

  if [ ! -e "${gpioEntry}" ]; then
    echo "GPIO ${gpio} entry ${gpioEntry} successfully disabled";
  else
    echo "***ERROR*** GPIO ${gpio} entry ${gpioEntry} still present";
    exit 1;
  fi
}

enable_gpio() {
  declare gpio="${1}";
  declare gpioEntry="${gpioDir}/gpio${gpio}";

  if [ -e "${gpioEntry}" ]; then
    disable_gpio ${gpio};
  fi

  if [ ! -e "${gpioEntry}" ]; then
    echo ${gpio} > "${gpioDir}/export";
    echo out > "${gpioEntry}/direction";
    echo 0 > "${gpioEntry}/active_low";
    echo none > "${gpioEntry}/edge";
    echo 0 > "${gpioEntry}/value";

    echo "GPIO ${gpio} entry ${gpioEntry} successfully enabled";
  else
    echo "***ERROR*** Failed to enable GPIO ${gpio} entry (${gpioEntry})";
    exit 1;
  fi
}

find_pwm() {
  declare pwm="${1}";
  foundPwmEntry="";
  for foundPwmEntry in ${pwmDir}/pwm_test_${pwm}*; do
    if [ -d "${foundPwmEntry}" ]; then
      return 0;
    fi
  done
  foundPwmEntry="";
  return 1;
}

enable_pwm() {
  declare pwm="${1}";

  # Make sure the pwm control overlay is loaded
  if ! grep -q ',am33xx_pwm' $SLOTS; then
    echo am33xx_pwm > ${SLOTS};
  fi

  if ! find_pwm ${pwm}; then
    # Try to enable specific PWM overlay
    echo bone_pwm_${pwm} > ${SLOTS};
  fi

  if find_pwm ${pwm}; then
    echo "PWM ${pwm} is enabled (${foundPwmEntry})";
    echo 0 > ${foundPwmEntry}/duty;
    echo ${period} > ${foundPwmEntry}/period;
    echo 0 > ${foundPwmEntry}/polarity;
    echo 1 > ${foundPwmEntry}/run;
    return 0;
  fi

  echo "***ERROR*** Failed to enable PWM ${pwm}";
  exit 1;
}

test_hbridge() {
  declare pwm="${1}";
  declare fwd="${2}";
  declare rev="${3}";
  declare disable="${4:-true}";
  declare fwdVal="${gpioDir}/gpio${fwd}/value";
  declare revVal="${gpioDir}/gpio${rev}/value";

  enable_pwm ${pwm};
  declare pwmEntry="${foundPwmEntry}";
  enable_gpio ${fwd};
  enable_gpio ${rev};

  for fwdState in 1 0; do
    echo $fwdState > ${fwdVal};
    for revState in 1 0; do
      echo $revState > ${revVal};
      for duty in 0 $((period / 2)) $period $((period / 2)) 0; do
	declare -i dutyPct=$((duty * 100 / period));
	echo $duty > ${pwmEntry}/duty;
        echo "Set duty=${dutyPct}% forward=${fwdState}, reverse=${revState}";
        sleep 1;
      done
    done
  done

  # Leave PWM configured, but turn off output signal
  echo 0 > ${pwmEntry}/run;

  if [ "${disableWhenDone}" == "true" ]; then
    disable_gpio ${fwd};
    disable_gpio ${rev};
  fi
}

if [ "${UID}" != 0 ]; then
  echo "***ERROR*** You must run this as root (sudo)"
  exit 1;
fi

test_hbridge P9_16 5 49;

test_hbridge P9_21 48 60;
