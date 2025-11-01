# ==============================================================================
#
#  Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# ==============================================================================
# ! /usr/bin/env python
from __future__ import absolute_import
from __future__ import print_function
from abc import ABCMeta, abstractmethod
import csv
import argparse
import sys
import os
import logging

from subprocess import check_output
from argparse import RawTextHelpFormatter
from .bm_config_restrictions import *
from .error import Error

logger = logging.getLogger(__name__)


class ArgsParser(object):
    def __init__(self, program_name, args_list):
        parser = argparse.ArgumentParser(
            prog=program_name,
            description="Run the {0}".format(BENCH_NAME),
            formatter_class=RawTextHelpFormatter)
        parser._action_groups.pop()
        required = parser.add_argument_group('required arguments')
        optional = parser.add_argument_group('optional arguments')
        required.add_argument('-c', '--config_file',
                              help='Path to a valid config file \nRefer to sample config file config_help.json '
                                   'for more detail on how to fill params in config file',
                              required=True)
        optional.add_argument('-o', '--output_base_dir_override',
                              help='Sets the output base directory.', required=False)
        optional.add_argument('-v', '--device_id_override',
                              help='Use this device ID instead of the one supplied in config file. '
                                   'Cannot be used with -a', required=False)
        optional.add_argument('-r', '--host_name',
                              help='Hostname/IP of remote machine to which devices are connected.',
                              required=False)
        optional.add_argument('-a', '--run_on_all_connected_devices_override', action='store_true',
                              help='Runs on all connected devices, currently only support 1. Cannot be used with -v',
                              required=False)
        optional.add_argument('-t', '--device_os_type_override',
                              help='Specify the target OS type, valid options are %s' % CONFIG_VALID_DEVICEOSTYPES,
                              required=False, default='android-aarch64')
        optional.add_argument('-d', '--debug', action='store_true',
                              help='Set to turn on debug log', required=False)
        optional.add_argument('-s', '--sleep', type=int, default=0,
                              help='Set number of seconds to sleep between runs e.g. 20 seconds',
                              required=False)
        optional.add_argument('-n', '--iterations', type=int,
                              help='Set the number of iterations to execute for calculating metrics',
                              required=False)
        optional.add_argument('-p', '--perfprofile', default='',
                              help='Set the benchmark operating mode (low_balanced, balanced, default, '
                                   'sustained_high_performance, high_performance, low_power_saver, power_saver, '
                                   'high_power_saver, system_settings, burst)', required=False)
        optional.add_argument('--backend_config', default='',
                              help='config file to specify context priority or provide backend extensions'
                                    'related parameters', required=False)
        optional.add_argument('-l', '--profilinglevel', default='',
                              help='Set the profiling level mode (basic, detailed). Default is basic.',
                              required=False)
        optional.add_argument('-json', '--generate_json', action='store_true',
                              help='Set to produce json output.', required=False)
        optional.add_argument('-be', '--backend', nargs='+', help='The backend to use',
                              required=False)
        optional.add_argument('--htp_serialized', action='store_true',
                              help='qnn graph prepare is done on x86 and execute is run on target',
                              required=False)
        optional.add_argument('--dsp_type', help='Specify DSP variant for QNN BM run',
                              required=False,
                              choices=['v65', 'v66', 'v68', 'v69', 'v69-plus', 'v73'], default=None)
        optional.add_argument('--arm_prepare', action='store_true',
                              help='qnn graph prepare is done on ARM and execute is run on target',
                              required=False)
        optional.add_argument('--use_signed_skel', action='store_true', required=False,
                              help='use signed skels for HTP runs')
        optional.add_argument('--enable_cache', action='store_true',
                            help='Enable caching mode to accelerate the network building process. Defaults to disable.', required=False)
        optional.add_argument('--shared_buffer', action='store_true',
                            help='Enables usage of shared buffer between application and backend for graph I/O', required=False)
        optional.add_argument('-clean_artifacts', action='store_true',
                            help='Clean the model specfic artifacts after inference', required=False)
        optional.add_argument('--cdsp_id', help='Specify which cdsp_id to use',
                              required=False,
                              choices=["0", "1"], default="0")

        self._args = vars(parser.parse_args([item for item in args_list]))

        if self._args['backend_config'] != '' and not self._args['backend_config'].startswith("/"):
            self._args['backend_config'] = os.path.abspath(self._args['backend_config'])

        if self._args['run_on_all_connected_devices_override'] and self._args['device_id_override']:
            print(
                'run_on_all_connected_devices_override (-a) and device_id_override (-v) are mutually exclusive')
            sys.exit(Error.ERRNUM_PARSEARGS_ERROR)
        return

    @property
    def use_signed_skel(self):
        return self._args['use_signed_skel']

    @property
    def config_file_path(self):
        return self._args['config_file']

    @property
    def htp_serialized(self):
        return self._args['htp_serialized']

    @property
    def dsp_type(self):
        return self._args['dsp_type']

    @property
    def cdsp_id(self):
        return self._args['cdsp_id']

    @property
    def enable_cache(self):
        return self._args['enable_cache']

    @property
    def clean_artifacts(self):
        return self._args['clean_artifacts']

    @property
    def arm_prepare(self):
        return self._args['arm_prepare']

    @property
    def debug_enabled(self):
        return self._args['debug']

    @property
    def sleep(self):
        return self._args['sleep']

    @property
    def perfprofile(self):
        if self._args['perfprofile'] is None:
            return ['high_performance']
        else:
            return self._args['perfprofile']

    @property
    def backend_config(self):
        return self._args['backend_config']

    @property
    def profilinglevel(self):
        return self._args['profilinglevel']

    @property
    def shared_buffer(self):
        return self._args['shared_buffer']

    @property
    def output_basedir_override(self):
        return self._args['output_base_dir_override']

    @property
    def run_on_all_connected_devices_override(self):
        return self._args['run_on_all_connected_devices_override']

    @property
    def device_id_override(self):
        if self._args['device_id_override'] is None:
            return []
        else:
            return self._args['device_id_override'].split(',')

    @property
    def iterations(self):
        return self._args['iterations']

    @property
    def host_name(self):
        return self._args['host_name']

    @property
    def device_os_type_override(self):
        return self._args['device_os_type_override']

    @property
    def args(self):
        return self._args

    @property
    def generate_json(self):
        return self._args['generate_json']

    @property
    def runtimes(self):
        return self._args['backend']


class DataFrame(object):
    def __init__(self):
        self._raw_data = []
        self._std_dev = []

    def __iter__(self):
        return self._raw_data.__iter__()

    def add_sum(self, channel, summation, length, runtime=""):
        # TODO: Figure out how to avoid putting in summation as max/min just to
        # satisfy unpacking
        self._raw_data.append(
            [channel, summation, length, summation, summation, runtime])

    def add_sum_max_min(self, channel, summation, length,
                        maximum, minimum, runtime):
        self._raw_data.append(
            [channel, summation, length, maximum, minimum, runtime])

    def add_std_dev(self, iteration, channel, stddev):
        self._std_dev.append([iteration, channel, stddev])


class AbstractLogParser(object):
    __metaclass__ = ABCMeta

    @abstractmethod
    def parse(self, input_dir):
        return "Derived class must implement this"


class ProductVersionParser(AbstractLogParser):
    def __init__(self, diagview_exe, product):
        self._diagview = diagview_exe
        self._product = product

    def __parse_diaglog(self, diag_log_file):
        try:
            diag_cmd = [self._diagview, '--input_log', diag_log_file]
            return check_output(diag_cmd).decode().split('\n')
        except Exception as de:
            logger.warning("Failed to parse {0}".format(diag_log_file))
            logger.warning(repr(de))
        return []

    def parse(self, input_dir):
        sdk_root = os.environ[self._product.SDK_ROOT]
        if sdk_root.endswith('/'):
            sdk_root = os.path.dirname(sdk_root)
        version_str = os.path.basename(sdk_root).replace('qnn-v', '')
        return version_str


class DroidTimingLogParser(AbstractLogParser):

    def __init__(self, model, diagview_exe,
                 model_info_exe, profilinglevel, product):
        self._model = model
        self._diagview = diagview_exe
        self._model_info = model_info_exe
        self._profilinglevel = profilinglevel
        self._product = product
        self.MAJOR_STATISTICS_1 = product.MAJOR_STATISTICS_1

    def __parse_diaglog(self, diag_log_file):
        try:
            output = []
            with open(diag_log_file, 'r') as f:
                for k in f.readlines():
                    output.append(k.strip())
            return output
        except Exception as de:
            logger.warning("Failed to parse {0}".format(diag_log_file))
            logger.warning(repr(de))
        return []

    def parse(self, input_dir):
        assert input_dir, 'ERROR: log_file is required'
        diag_log_file = os.path.join(
            input_dir, self._product.PARSE_OUT_FILE)
        diag_log_output = self.__parse_diaglog(diag_log_file)

        data_frame = DataFrame()
        line = 0
        while line < len(diag_log_output):
            data = diag_log_output[line]
            statistic_key = data.split(":")[0]
            if statistic_key in self.MAJOR_STATISTICS_1:
                stat_line = line + 2
                while stat_line < len(diag_log_output) and diag_log_output[stat_line].split(":")[0] \
                        not in self.MAJOR_STATISTICS_1 and diag_log_output[stat_line] != '':
                    statistic_data = diag_log_output[stat_line].split(': ')
                    try:
                        statistic_time = int(statistic_data[1].split()[0])
                    except:
                        stat_line = stat_line + 1
                        continue
                    source = statistic_data[0].replace(' ', '').replace('GRAPH_EXECUTE_', ''). \
                        replace('GRAPH_FINALIZE_', '').replace('QnnGraph_finalize',
                                                               'GRAPH_FINALIZE'). \
                        replace('Graph::execute', 'GRAPH_EXECUTE').replace('QnnGraph_execute',
                                                                           'GRAPH_EXECUTE')
                    data_frame.add_sum(statistic_key + ' [' + source + ']', statistic_time, 1)
                    stat_line = stat_line + 1
                line = stat_line
            else:
                line = line + 1

        diaglog_csv = os.path.join(input_dir, self._product.BENCH_DIAG_CSV_FILE)
        per_image_stats = dict()
        with open(diaglog_csv, 'r') as f:
            csv_reader = csv.reader(f, delimiter=',')
            for row in csv_reader:
                try:
                    value = int(row[2])
                except:
                    continue
                if row[1] == "EXECUTE":
                    if row[3] == 'US':
                        k = "Total Inference Time [{}{}]".format(
                            row[4].title().replace("Netrun", "NetRun"),
                            "" if row[6] == "null" else "({})".format(
                                row[6].replace(" ", "").replace(
                                    'QnnGraph_execute', 'GRAPH_EXECUTE')))
                    else:
                        k = "Total Inference Time [{}]".format(
                            "" if row[6] == "null" else row[6].replace(" ", "").replace(
                                'QnnGraph_execute', 'GRAPH_EXECUTE'))
                    if k not in per_image_stats.keys():
                        per_image_stats[k] = []
                    per_image_stats[k].append(value)
        return data_frame, per_image_stats


class DroidDumpSysMemLogParser(AbstractLogParser):
    PSS = 'pss'
    PRV_DIRTY = 'prv_dirty'
    PRV_CLEAN = 'prv_clean'

    def __init__(self, product):
        self._product = product
        pass

    def parse(self, input_dir):
        assert input_dir, 'ERROR: log_file is required'
        log_file = os.path.join(input_dir, self._product.MEM_LOG_FILE_NAME)
        fid = open(log_file, 'r')
        key_word = 'TOTAL'

        # to get around addn information being dumped in Android M
        skip_key_word = 'PSS:'
        pss = []
        prv_dirty = []
        prv_clean = []

        logger.debug('Parsing Memory Log: %s' % log_file)
        for line in fid.readlines():
            tmp = line.strip().split()
            if (key_word in tmp) and not (skip_key_word in tmp):
                _pss = int(tmp[1])
                _prv_dirty = int(tmp[2])
                _prv_clean = int(tmp[3])
                if _pss == 0 and _prv_dirty == 0 and _prv_clean == 0:
                    continue
                pss.append(int(tmp[1]))
                prv_dirty.append(int(tmp[2]))
                prv_clean.append(int(tmp[3]))

        data_frame = DataFrame()
        if len(pss) == 0 and len(prv_dirty) == 0 and len(prv_clean) == 0:
            logger.warning('No memory info found in %s' % log_file)
        else:
            data_frame.add_sum_max_min(
                self.PSS, sum(pss), len(pss), max(pss), min(pss))
            data_frame.add_sum_max_min(
                self.PRV_DIRTY,
                sum(prv_dirty),
                len(prv_dirty),
                max(prv_dirty),
                min(prv_dirty))
            data_frame.add_sum_max_min(
                self.PRV_CLEAN,
                sum(prv_clean),
                len(prv_clean),
                max(prv_clean),
                min(prv_clean))
        return data_frame


class LogParserFactory(object):
    def __init__(self):
        pass

    @staticmethod
    def make_parser(measure, config, product):
        if measure == product.MEASURE_MEM:
            if config.platform == product.PLATFORM_OS_ANDROID:
                return DroidDumpSysMemLogParser(product)
            else:
                raise Exception(
                    "make_parser: Invalid platform !!!",
                    config.platform)
        elif measure == product.MEASURE_TIMING:
            return DroidTimingLogParser(config.dnn_model.model,
                                        config.host_artifacts[product.DIAGVIEW_EXE],
                                        config.host_artifacts[product.MODEL_INFO_EXE],
                                        config.profilinglevel,
                                        product)

        elif measure == product.MEASURE_PRODUCT_VERSION:
            return ProductVersionParser(
                config.host_artifacts[product.DIAGVIEW_EXE], product)
