import os
import serial.tools.list_ports
import can
import rospy
from std_msgs.msg import String
from sensor_msgs.msg import JointState
from geometry_msgs.msg import TransformStamped
import math


def update_joint_sliders(data):
    """ Update joint sliders based on received joint state data """
    global last_joint_states
    # Check if the received joint states are different from the last ones
    if last_joint_states is None or data.position != last_joint_states.position:
        last_joint_states = data
        # Update joint sliders and labels here
        for i, joint in enumerate(data.name):
            if joint in joint_name_to_slider:
                slider = joint_name_to_slider[joint]
                slider.set(data.position[i])

def update_cartesian_sliders(data):
    """ Update Cartesian sliders based on received transformed tf data """
    global last_cartesian_position
    # Check if the received Cartesian position is different from the last one
    if last_cartesian_position is None or \
        data.transform.translation.x != last_cartesian_position.transform.translation.x or \
        data.transform.translation.y != last_cartesian_position.transform.translation.y or \
        data.transform.translation.z != last_cartesian_position.transform.translation.z:
        last_cartesian_position = data
        # Update Cartesian sliders and labels here
        cartesian_path_sliders[0].set(data.transform.translation.x)
        cartesian_path_sliders[1].set(data.transform.translation.y)
        cartesian_path_sliders[2].set(data.transform.translation.z)

# Function to run the ROS script
def run_ros_script():
    os.system("python3 rosjog.py")
  

# Function to go to joint state
def go_to_joint_state():
    joint_state_values = [slider.get() for slider in joint_state_sliders]
    ui_command_pub.publish("go_to_joint_state," + ','.join(map(str, joint_state_values)))

# Function to plan cartesian path
def plan_cartesian_path():
    cartesian_path_values = [slider.get() for slider in cartesian_path_sliders]
    ui_command_pub.publish("plan_cartesian_path," + ','.join(map(str, cartesian_path_values)))

def open_gripper():
    ui_command_pub.publish("open_gripper")

def close_gripper():
    ui_command_pub.publish("close_gripper")

# Function to send the data to gcode.txt
def send_data():
    # Get values from the entry fields
    address = address_entry.get()
    data_fields = [entry.get() for entry in data_entries]
    
    # Combine all values into one string
    combined_data = address + ''.join(data_fields)
    
    # Write combined data to "gcode.txt"
    try:
        with open("gcode.txt", "w") as gcode_file:
            gcode_file.write(combined_data)
        update_message("Data written to 'gcode.txt' successfully.")
    except Exception as e:
        update_message(f"Error writing to 'gcode.txt': {e}")
    os.system("python3 send.py")


# Joint State Pose; Used to position the Robot by manually adjsuting te joints
joint_state_values = [] 

# clear the file
open('jog.tap', 'w').close() 

# ROS initialization
rospy.init_node('control_gui', anonymous=True)
ui_command_pub = rospy.Publisher('/ui_command', String, queue_size=10)
joint_states_sub = rospy.Subscriber('/joint_states', JointState, update_joint_sliders)
transformed_tf_sub = rospy.Subscriber('/transformed_tf', TransformStamped, update_cartesian_sliders)

# Configure row and column weights for resizing
for i in range(20):
    root.rowconfigure(i, weight=1)
for i in range(4):
    root.columnconfigure(i, weight=1)

# Set theme light/dark
sv_ttk.set_theme("dark")

root.mainloop()
