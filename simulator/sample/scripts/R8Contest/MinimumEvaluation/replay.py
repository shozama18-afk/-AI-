# Copyright (c) 2021-2025 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
import os
import json
from argparse import ArgumentParser
from ASRCAISim1.core import Factory
from ASRCAISim1.viewer.GodViewLoader import GodViewLoader
from r8_contest_models.R8ContestGodView import R8ContestGodView,R8ContestGodViewPanel

def parse_args():
    parser = ArgumentParser()
    parser.add_argument('--movie-dir', default='./log')
    parser.add_argument('--as-video', type=int, default=1)
    parser.add_argument("-u","--ui-config-file",type=str,default="",help="ui config json file")
    parser.add_argument("-y","--youth",action="store_true",help="use when you want to replay with GodView for youth-division")

    return parser.parse_args()


def main():
    args = parse_args()
    _, _, modelConfig, _ = Factory().resolveModelConfig("Viewer","r8_contest_models.R8Contest"+("Youth" if args.youth else "Open")+"God")
    if args.ui_config_file != '' and os.path.exists(args.ui_config_file):
        with open(args.ui_config_file,'r') as f:
            modelConfig.merge_patch(json.load(f))
    modelConfig.merge_patch({
        "globPattern":os.path.join(args.movie_dir, "**.dat"),
        "outputDir":args.movie_dir,
        "outputFileNamePrefix":"",
        "asVideo":args.as_video, # Trueだと動画(mp4)、Falseだと連番画像（png）として保存
        "fps":60,
    })
    GodViewLoader.defaultPanelConfigPath=R8ContestGodView.defaultPanelConfigPath
    GodViewLoader.panelClass=R8ContestGodViewPanel
    loader=GodViewLoader(modelConfig)
    loader.run()

if __name__ == "__main__":
    main()
