#include <vigir_footstep_planning_lib/modeling/state.h>

namespace vigir_footstep_planning
{
State::State()
  : ivLeg(NOLEG)
  , ivRoll(0.0)
  , ivPitch(0.0)
  , ivYaw(0.0)
  , ivSwingHeight(0.0)
  , ivSwayDuration(0.0)
  , ivStepDuration(0.0)
  , ivGroundContactSupport(0.0)
  , v_body(0.0)
  , cost(0.0)
  , risk(0.0)
{
  ivPose = tf::Pose();

  ivNormal.x = 0.0;
  ivNormal.y = 0.0;
  ivNormal.z = 1.0;
}

State::State(double x, double y, double z, double roll, double pitch, double yaw, double swing_height, double sway_duration, double step_duration, Leg leg)
  : ivLeg(leg)
  , ivRoll(roll)
  , ivPitch(pitch)
  , ivYaw(yaw)
  , ivSwingHeight(swing_height)
  , ivSwayDuration(sway_duration)
  , ivStepDuration(step_duration)
  , ivGroundContactSupport(1.0)
  , v_body(0.0)
  , cost(0.0)
  , risk(0.0)
{
  ivPose.setOrigin(tf::Vector3(x, y, z));
  ivPose.getBasis().setRPY(ivRoll, ivPitch, ivYaw);

  recomputeNormal();
}

State::State(const geometry_msgs::Vector3& position, double roll, double pitch, double yaw, double swing_height, double sway_duration, double step_duration, Leg leg)
  : State(position.x, position.y, position.z, roll, pitch, yaw, swing_height, sway_duration, step_duration, leg)
{
}

State::State(const geometry_msgs::Vector3& position, const geometry_msgs::Vector3& normal, double yaw, double swing_height, double sway_duration, double step_duration, Leg leg)
  : ivLeg(leg)
  , ivYaw(yaw)
  , ivSwingHeight(swing_height)
  , ivSwayDuration(sway_duration)
  , ivStepDuration(step_duration)
  , ivGroundContactSupport(1.0)
  , v_body(0.0)
  , cost(0.0)
  , risk(0.0)
{
  ivPose.setOrigin(tf::Vector3(position.x, position.y, position.z));

  setNormal(normal);
}

State::State(const geometry_msgs::Pose& pose, double swing_height, double sway_duration, double step_duration, Leg leg)
  : ivLeg(leg)
  , ivSwingHeight(swing_height)
  , ivSwayDuration(sway_duration)
  , ivStepDuration(step_duration)
  , ivGroundContactSupport(1.0)
  , v_body(0.0)
  , cost(0.0)
  , risk(0.0)
{
  tf::poseMsgToTF(pose, ivPose);
  ivPose.getBasis().getRPY(ivRoll, ivPitch, ivYaw);

  recomputeNormal();
}

State::State(const tf::Transform& t, double swing_height, double sway_duration, double step_duration, Leg leg)
  : ivLeg(leg)
  , ivSwingHeight(swing_height)
  , ivSwayDuration(sway_duration)
  , ivStepDuration(step_duration)
  , ivGroundContactSupport(1.0)
  , v_body(0.0)
  , cost(0.0)
  , risk(0.0)
{
  ivPose = t;
  ivPose.getBasis().getRPY(ivRoll, ivPitch, ivYaw);

  recomputeNormal();
}

State::State(const msgs::Foot foot, double swing_height, double sway_duration, double step_duration)
  : State(foot.pose, swing_height, sway_duration, step_duration, foot.foot_index == msgs::Foot::LEFT ? LEFT : RIGHT)
{
}

State::State(const msgs::Step step)
  : State(step.foot, step.swing_height, step.sway_duration, step.step_duration)
{
}

State::~State()
{}

bool State::operator==(const State& s2) const
{
  return (fabs(getX() - s2.getX()) < FLOAT_CMP_THR &&
          fabs(getY() - s2.getY()) < FLOAT_CMP_THR &&
          fabs(getZ() - s2.getZ()) < FLOAT_CMP_THR &&
          fabs(angles::shortest_angular_distance(ivRoll, s2.ivRoll)) < FLOAT_CMP_THR &&
          fabs(angles::shortest_angular_distance(ivPitch, s2.ivPitch)) < FLOAT_CMP_THR &&
          fabs(angles::shortest_angular_distance(ivYaw, s2.ivYaw)) < FLOAT_CMP_THR &&
          ivLeg == s2.getLeg());
}

bool State::operator!=(const State& s2) const
{
  return not (*this == s2);
}

void State::setPosition(const geometry_msgs::Point& position)
{
  setX(position.x);
  setY(position.y);
  setZ(position.z);
}

void State::setOrientation(const geometry_msgs::Quaternion& q)
{
  double roll, pitch, yaw;
  tf::Quaternion q_tf;
  tf::quaternionMsgToTF(q, q_tf);
  tf::Matrix3x3(q_tf).getRPY(roll, pitch, yaw);
  setRPY(roll, pitch, yaw);
}

void State::setRPY(double roll, double pitch, double yaw)
{
  ivRoll = roll;
  ivPitch = pitch;
  ivYaw = yaw;

  ivPose.getBasis().setRPY(ivRoll, ivPitch, ivYaw);
  recomputeNormal();
}

void State::setNormal(const geometry_msgs::Vector3 &normal)
{
  ivNormal = normal;

  if (ivNormal.z > 0.99)
  {
    ivNormal.x = 0.0;
    ivNormal.y = 0.0;
    ivNormal.z = 1.0;
    ivRoll = 0.0;
    ivPitch = 0.0;
  }
  else
  {
    // get roll and pitch
    normalToRP(ivNormal, ivYaw, ivRoll, ivPitch);
  }

  ivPose.getBasis().setRPY(ivRoll, ivPitch, ivYaw);
}

void State::setNormal(double x, double y, double z)
{
  geometry_msgs::Vector3 n;
  n.x = x;
  n.y = y;
  n.z = z;
  setNormal(n);
}

void State::getStep(msgs::Step &step) const
{
  step.step_index = 0;
  step.foot.foot_index = ivLeg;
  getFoot(step.foot);
  step.sway_duration = ivSwayDuration;
  step.step_duration = ivStepDuration;
  step.swing_height = ivSwingHeight;
  step.cost = cost;
  step.risk = risk;
}

void State::getFoot(msgs::Foot& foot) const
{
  foot.pose.position.x = getX();
  foot.pose.position.y = getY();
  foot.pose.position.z = getZ();
  normalToQuaternion(ivNormal, ivYaw, foot.pose.orientation);
}

void State::recomputeNormal()
{
  if (std::abs(ivRoll) < 0.01 && std::abs(ivPitch) < 0.01)
  {
    ivNormal.x = 0.0;
    ivNormal.y = 0.0;
    ivNormal.z = 1.0;
  }
  else
  {
    // get normal
    RPYToNormal(ivRoll, ivPitch, ivYaw, ivNormal);
  }
}

} // end of namespace
