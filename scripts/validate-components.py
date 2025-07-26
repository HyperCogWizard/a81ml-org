#!/usr/bin/env python3
"""
Component Dependency Validator

This script validates component dependencies and enforces requirements
for the cognitive architecture components.
"""

import os
import sys
import yaml
import argparse
from pathlib import Path
from typing import Dict, List, Set, Optional


class ComponentRegistry:
    """Registry for managing component specifications and dependencies."""
    
    def __init__(self, components_dir: str):
        self.components_dir = Path(components_dir)
        self.components: Dict[str, dict] = {}
        self.load_components()
    
    def load_components(self):
        """Load all component specifications from the components directory."""
        if not self.components_dir.exists():
            print(f"Components directory not found: {self.components_dir}")
            return
        
        for yaml_file in self.components_dir.glob("*.yaml"):
            with open(yaml_file, 'r') as f:
                try:
                    component = yaml.safe_load(f)
                    if component and 'name' in component:
                        self.components[component['name']] = component
                        print(f"Loaded component: {component['name']}")
                    else:
                        print(f"Invalid component file: {yaml_file}")
                except yaml.YAMLError as e:
                    print(f"Error loading {yaml_file}: {e}")
    
    def get_component(self, name: str) -> Optional[dict]:
        """Get component specification by name."""
        return self.components.get(name)
    
    def list_components(self) -> List[str]:
        """List all available components."""
        return list(self.components.keys())
    
    def validate_dependencies(self) -> bool:
        """Validate all component dependencies."""
        all_valid = True
        
        for name, component in self.components.items():
            if not self._validate_component_dependencies(name, component):
                all_valid = False
        
        return all_valid
    
    def _validate_component_dependencies(self, name: str, component: dict) -> bool:
        """Validate dependencies for a single component."""
        requires = component.get('requires', [])
        
        print(f"\nValidating component '{name}':")
        print(f"  Type: {component.get('type', 'unknown')}")
        print(f"  Requires: {requires if requires else 'none'}")
        
        if not requires:
            print(f"  ✓ No dependencies")
            return True
        
        valid = True
        for dep in requires:
            if dep not in self.components:
                print(f"  ✗ Missing dependency: {dep}")
                valid = False
            else:
                print(f"  ✓ Dependency satisfied: {dep}")
        
        return valid
    
    def check_circular_dependencies(self) -> bool:
        """Check for circular dependencies."""
        print("\nChecking for circular dependencies...")
        
        visited = set()
        rec_stack = set()
        
        def has_cycle(component_name: str) -> bool:
            if component_name in rec_stack:
                return True
            if component_name in visited:
                return False
            
            visited.add(component_name)
            rec_stack.add(component_name)
            
            component = self.components.get(component_name)
            if component:
                for dep in component.get('requires', []):
                    if has_cycle(dep):
                        return True
            
            rec_stack.remove(component_name)
            return False
        
        for component_name in self.components:
            if component_name not in visited:
                if has_cycle(component_name):
                    print(f"  ✗ Circular dependency detected involving: {component_name}")
                    return False
        
        print("  ✓ No circular dependencies found")
        return True
    
    def get_dependency_order(self) -> List[str]:
        """Get components in dependency order (topological sort)."""
        visited = set()
        temp_mark = set()
        result = []
        
        def visit(component_name: str):
            if component_name in temp_mark:
                raise ValueError(f"Circular dependency detected: {component_name}")
            if component_name in visited:
                return
            
            temp_mark.add(component_name)
            
            component = self.components.get(component_name)
            if component:
                for dep in component.get('requires', []):
                    if dep in self.components:
                        visit(dep)
            
            temp_mark.remove(component_name)
            visited.add(component_name)
            result.append(component_name)
        
        for component_name in self.components:
            if component_name not in visited:
                visit(component_name)
        
        return result
    
    def validate_file_existence(self) -> bool:
        """Validate that source files exist for all components."""
        print("\nValidating source file existence...")
        all_exist = True
        
        for name, component in self.components.items():
            source = component.get('source', {})
            header = source.get('header')
            implementation = source.get('implementation')
            
            print(f"\nComponent '{name}':")
            
            if header:
                header_path = Path(header)
                if header_path.exists():
                    print(f"  ✓ Header file exists: {header}")
                else:
                    print(f"  ✗ Header file missing: {header}")
                    all_exist = False
            
            if implementation:
                impl_path = Path(implementation)
                if impl_path.exists():
                    print(f"  ✓ Implementation file exists: {implementation}")
                else:
                    print(f"  ✗ Implementation file missing: {implementation}")
                    all_exist = False
        
        return all_exist


def main():
    parser = argparse.ArgumentParser(description="Validate component dependencies")
    parser.add_argument("--components-dir", default="components",
                       help="Directory containing component specifications")
    parser.add_argument("--list", action="store_true",
                       help="List all available components")
    parser.add_argument("--validate", action="store_true",
                       help="Validate all component dependencies")
    parser.add_argument("--check-files", action="store_true",
                       help="Check if source files exist")
    parser.add_argument("--dependency-order", action="store_true",
                       help="Show components in dependency order")
    
    args = parser.parse_args()
    
    registry = ComponentRegistry(args.components_dir)
    
    if args.list:
        print("Available components:")
        for component in sorted(registry.list_components()):
            comp = registry.get_component(component)
            print(f"  {component}: {comp.get('description', 'No description')}")
        return
    
    if args.dependency_order:
        print("Components in dependency order:")
        try:
            order = registry.get_dependency_order()
            for i, component in enumerate(order, 1):
                comp = registry.get_component(component)
                requires = comp.get('requires', [])
                req_str = f" (requires: {', '.join(requires)})" if requires else ""
                print(f"  {i}. {component}{req_str}")
        except ValueError as e:
            print(f"Error: {e}")
            return 1
        return
    
    success = True
    
    if args.validate or not any([args.list, args.check_files, args.dependency_order]):
        print("=== Component Dependency Validation ===")
        if not registry.validate_dependencies():
            success = False
        
        if not registry.check_circular_dependencies():
            success = False
    
    if args.check_files:
        if not registry.validate_file_existence():
            success = False
    
    if success:
        print("\n✓ All validations passed!")
        return 0
    else:
        print("\n✗ Validation failed!")
        return 1


if __name__ == "__main__":
    sys.exit(main())