import argparse
import jinja2
import os
import sys
import pathlib
from glorpen.config import Config as GConfig
import glorpen.config.loaders as loaders
import glorpen.config.fields.simple as fields_simple
import glorpen.config.fields.version as fields_version
import re
from jinja2 import nodes
from jinja2.ext import Extension
import datetime
import semver

re_line = re.compile("(\s*\n\s*)+")
def filter_oneline(value):
    return re_line.sub(" ", value.strip())

class EmbedExtension(Extension):
    tags = set(['embed'])

    def parse(self, parser):
        lineno = next(parser.stream).lineno
        tpl_name = parser.parse_expression()
    
        tpl_data, file, updater = self.environment.loader.get_source(self.environment, tpl_name.value)
        return self.environment.parse(tpl_data).body

install_dir = "/opt/puppetizer"

class Config(object):
    
    _pkg_keys = ('puppet', 'facter', 'ruby', 'leatherman', 'cpp-hocon', 'boost', 'yaml-cpp', 'runit')
    # hiera5 is included in puppet
    
    def __init__(self, conf_path):
        super(Config, self).__init__()
        
        self._ver_keys = ['ruby_gems', 'ruby_bundler', 'hiera']
        self._ver_keys.extend(self._pkg_keys)
        
        cfg = self._load(conf_path)
        
        self._checksums = cfg.get("sha256")
        self._packages = cfg.get("package_sets")
        self._targets = cfg.get("builds")["targets"]
        self._build_version = cfg.get("builds")["version"]
    
    def _load(self, config_path):
        loader = loaders.YamlLoader(filepath=config_path)
        spec = fields_simple.Dict({
            "sha256": fields_simple.Dict(dict(((i, fields_simple.Dict(keys=fields_simple.String(), values=fields_simple.String()))) for i in self._pkg_keys)),
            "package_sets": fields_simple.Dict(
                keys = fields_simple.String(),
                values = fields_simple.Dict(dict(((i, fields_version.Version())) for i in self._ver_keys))
            ),
            "builds": fields_simple.Dict({
                "version": fields_version.Version(),
                "targets": fields_simple.Dict(
                    keys = fields_simple.String(),
                    values = fields_simple.Dict({
                        "source-image": fields_simple.String(),
                        "system": fields_simple.String(),
                        "system-version": fields_version.Version(),
                        "puppet-package-version": fields_simple.String(),
                        "system-packages": fields_simple.List(fields_simple.String())
                    })
                )
            })
        })
        return GConfig(loader=loader, spec=spec).finalize()

    def __getitem__(self, name):
        
        s = self._targets[name]
        pkg = {}
        
        for k,v in self._packages[s["puppet-package-version"]].items():
            pkg[k] = {
                "version": v
            }
            if k in self._checksums:
                pkg[k]["sha256"] = self._checksums[k][str(v)]
        
        ret = {
            "puppet_package": pkg,
            "install_dir": install_dir,
            "system": s["system"],
            "system_version": s["system-version"],
            "system_packages": s["system-packages"],
            "source_image": s["source-image"],
            "version": self.version
        }
        return ret
    
    @property
    def targets(self):
        return tuple(self._targets.keys())
    @property
    def version(self):
        return self._build_version


class Renderer(object):
    
    re_empty_lines = re.compile('\n+')
    
    def __init__(self):
        super(Renderer, self).__init__()
        
        self._root_dir = pathlib.Path(__file__).parent
        
        self.env = jinja2.Environment(
            loader=jinja2.FileSystemLoader(self._root_dir.as_posix()),
            extensions=[EmbedExtension]
        )
        self.env.filters["oneline"] = filter_oneline
        self.env.globals.update(
            now=datetime.datetime.utcnow(),
            v=semver.parse_version_info
        )
    
    def render(self, name):
        cfg = self.config[name]
        tpl = self.env.get_template("%s.dockerfile.jinja2" % cfg["system"])
        return self.re_empty_lines.sub("\n", tpl.render(cfg))

    def load_config(self, config_path):
        self.config = Config((self._root_dir / config_path).as_posix())

def cli_render(r, ns):
    sys.stdout.write(r.render(ns.name))

def cli_info(r, ns):
    if ns.mode == "targets":
        sys.stdout.write("\n".join(r.config.targets)+"\n")
    elif ns.mode == "version":
        sys.stdout.write(str(r.config.version)+"\n")

if __name__ == "__main__":
    p = argparse.ArgumentParser()
    subparsers = p.add_subparsers()

    p_render = subparsers.add_parser("render")
    p_render.set_defaults(f=cli_render)
    p_render.add_argument('name')

    p_info = subparsers.add_parser("info")
    p_info.set_defaults(f=cli_info)
    p_info.add_argument("mode", choices=('targets','version'))
    
    ns = p.parse_args()
    
    r = Renderer()
    r.load_config("config.yaml")
    ns.f(r, ns)
