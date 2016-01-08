#!/usr/bin/env python

import rospy
import std_msgs.msg
import vigir_footstep_planning_msgs.msg

from rqt_gui_py.plugin import Plugin
from python_qt_binding.QtCore import Qt
from python_qt_binding.QtGui import QWidget, QHBoxLayout, QVBoxLayout, QPushButton, QComboBox

from vigir_footstep_planning_msgs.msg import StepPlan, ExecuteStepPlanAction, ExecuteStepPlanGoal, ErrorStatus
from vigir_footstep_planning_lib.execute_step_plan_widget import *
from vigir_footstep_planning_lib.error_status_widget import *
from vigir_footstep_planning_lib.logging import *
from vigir_footstep_planning_lib.qt_helper import *


class ExecuteStepPlanDialog(Plugin):

    def __init__(self, context):
        super(ExecuteStepPlanDialog, self).__init__(context)
        self.setObjectName('ExecuteStepPlanDialog')

        self._parent = QWidget()
        self._widget = ExecuteStepPlanWidget(self._parent)

        context.add_widget(self._parent)

    def shutdown_plugin(self):
        self._widget.shutdown_plugin()


class ExecuteStepPlanWidget(QObject):

    execute_step_plan_client = None
    step_plan = None

    def __init__(self, context):
        super(ExecuteStepPlanWidget, self).__init__()

        # start widget
        widget = context
        error_status_widget = QErrorStatusWidget()
        self.logger = Logger(error_status_widget)

        # add upper part
        vbox = QVBoxLayout()

        # add execute action server widget
        add_widget_with_frame(vbox, QExecuteStepPlanWidget(logger=self.logger), "Step Plan Selection/Execution:")

        # add error status widget
        add_widget_with_frame(vbox, error_status_widget, "Status:")

        # end widget
        widget.setLayout(vbox)
        #context.add_widget(widget)

    def shutdown_plugin(self):
        print "Shutting down ..."
        print "Done!"
