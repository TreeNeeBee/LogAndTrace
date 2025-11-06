#!/usr/bin/env python3
"""
Message Catalog Generator for LightAP Modeled Messages

Scans source code for MessageId definitions and generates a JSON catalog
that can be used by log analysis tools to decode modeled messages.

Usage:
    ./generate_message_catalog.py <source_dir> [output_file]

Example:
    ./generate_message_catalog.py modules/LogAndTrace/source/inc message_catalog.json
"""

import re
import json
import sys
import argparse
from pathlib import Path
from datetime import datetime

def extract_message_ids(file_path):
    """
    Extract MessageId definitions from a C++ header file.
    
    Looks for patterns like:
        struct StartupMessage : MessageId<1000> {};
    
    Returns list of (name, id, line_number) tuples
    """
    messages = []
    
    # Pattern: struct NAME : MessageId<ID> {};
    pattern = re.compile(r'struct\s+(\w+)\s*:\s*MessageId<(\d+)>\s*\{', re.MULTILINE)
    
    # Pattern for documentation comments
    doc_pattern = re.compile(r'/\*\*\s*(.*?)\s*\*/', re.DOTALL)
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
            lines = content.split('\n')
            
            for match in pattern.finditer(content):
                name = match.group(1)
                msg_id = match.group(2)
                line_num = content[:match.start()].count('\n') + 1
                
                # Try to find documentation comment before this line
                doc = ""
                for i in range(max(0, line_num - 10), line_num):
                    if i < len(lines):
                        doc_match = doc_pattern.search(lines[i])
                        if doc_match:
                            doc = doc_match.group(1).strip()
                            break
                
                messages.append({
                    'name': name,
                    'id': msg_id,
                    'line': line_num,
                    'file': str(file_path),
                    'description': doc
                })
    
    except Exception as e:
        print(f"Warning: Failed to parse {file_path}: {e}", file=sys.stderr)
    
    return messages

def scan_directory(directory):
    """
    Recursively scan directory for C++ headers containing MessageId definitions.
    
    Returns dict of {message_id: message_info}
    """
    messages = {}
    
    dir_path = Path(directory)
    if not dir_path.exists():
        print(f"Error: Directory not found: {directory}", file=sys.stderr)
        return messages
    
    # Scan all .hpp and .h files
    for pattern in ['**/*.hpp', '**/*.h']:
        for file_path in dir_path.glob(pattern):
            file_messages = extract_message_ids(file_path)
            for msg in file_messages:
                msg_id = msg['id']
                
                # Check for duplicate IDs
                if msg_id in messages:
                    print(f"Warning: Duplicate message ID {msg_id}: "
                          f"{messages[msg_id]['name']} vs {msg['name']}", 
                          file=sys.stderr)
                
                messages[msg_id] = msg
    
    return messages

def extract_trace_switch_config(file_path):
    """
    Extract TraceSwitch specializations to determine routing configuration.
    
    Looks for patterns like:
        template<>
        struct TraceSwitch<StartupMessage> {
            static constexpr TraceRoute route = TraceRoute::Both;
        };
    """
    routing = {}
    
    pattern = re.compile(
        r'template<>\s*struct\s+TraceSwitch<(\w+)>\s*\{[^}]*'
        r'TraceRoute\s+route\s*=\s*TraceRoute::(\w+)',
        re.MULTILINE | re.DOTALL
    )
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
            for match in pattern.finditer(content):
                msg_name = match.group(1)
                route = match.group(2)
                routing[msg_name] = route
    except Exception as e:
        print(f"Warning: Failed to parse {file_path}: {e}", file=sys.stderr)
    
    return routing

def scan_trace_switches(directory):
    """Scan directory for TraceSwitch configurations"""
    routing = {}
    
    dir_path = Path(directory)
    for pattern in ['**/*.hpp', '**/*.h']:
        for file_path in dir_path.glob(pattern):
            file_routing = extract_trace_switch_config(file_path)
            routing.update(file_routing)
    
    return routing

def generate_catalog(messages, routing, output_file):
    """
    Generate message catalog JSON file.
    
    Format:
    {
        "version": "1.0",
        "application": "LightAP",
        "timestamp": "2025-11-06T12:00:00Z",
        "messages": {
            "1000": {
                "name": "StartupMessage",
                "id": 1000,
                "description": "Application startup event",
                "routing": "Both",
                "source_file": "ExampleMessages.hpp",
                "source_line": 42
            }
        }
    }
    """
    catalog = {
        "version": "1.0",
        "application": "LightAP",
        "generated_at": datetime.now().isoformat(),
        "total_messages": len(messages),
        "messages": {}
    }
    
    for msg_id, msg_info in sorted(messages.items(), key=lambda x: int(x[0])):
        msg_name = msg_info['name']
        
        catalog["messages"][msg_id] = {
            "id": int(msg_id),
            "name": msg_name,
            "description": msg_info.get('description', ''),
            "routing": routing.get(msg_name, 'Logger'),  # Default: Logger
            "source": {
                "file": msg_info['file'],
                "line": msg_info['line']
            },
            "format": f"Message {msg_id} - {msg_name}",  # Can be enhanced later
            "parameters": []  # Can be extracted from usage patterns
        }
    
    # Write to file
    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(catalog, f, indent=2, ensure_ascii=False)
    
    return catalog

def main():
    parser = argparse.ArgumentParser(
        description='Generate message catalog from source code',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Example usage:
  %(prog)s modules/LogAndTrace/source/inc
  %(prog)s modules/LogAndTrace/source/inc message_catalog.json
  %(prog)s . --verbose
        """
    )
    
    parser.add_argument('source_dir', 
                       help='Source directory to scan for MessageId definitions')
    parser.add_argument('output', 
                       nargs='?',
                       default='message_catalog.json',
                       help='Output JSON file (default: message_catalog.json)')
    parser.add_argument('-v', '--verbose',
                       action='store_true',
                       help='Verbose output')
    
    args = parser.parse_args()
    
    if args.verbose:
        print(f"Scanning directory: {args.source_dir}")
    
    # Scan for MessageId definitions
    messages = scan_directory(args.source_dir)
    
    if args.verbose:
        print(f"Found {len(messages)} message definitions")
    
    # Scan for TraceSwitch configurations
    routing = scan_trace_switches(args.source_dir)
    
    if args.verbose:
        print(f"Found {len(routing)} TraceSwitch configurations")
    
    # Generate catalog
    catalog = generate_catalog(messages, routing, args.output)
    
    print(f"✓ Generated message catalog: {args.output}")
    print(f"  Total messages: {catalog['total_messages']}")
    
    # Print summary
    if args.verbose:
        print("\nMessage Summary:")
        for msg_id in sorted(catalog['messages'].keys(), key=int):
            msg = catalog['messages'][msg_id]
            print(f"  [{msg_id:4}] {msg['name']:30} (routing: {msg['routing']})")

if __name__ == '__main__':
    main()
