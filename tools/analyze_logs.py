#!/usr/bin/env python3
"""
Log Analyzer for LightAP Modeled Messages

Analyzes log files containing modeled messages and restores full message text
using the message catalog.

Usage:
    ./analyze_logs.py <message_catalog.json> <logfile.log>

Example:
    ./analyze_logs.py message_catalog.json /var/log/lightap.log
"""

import re
import json
import sys
import argparse
from pathlib import Path
from collections import defaultdict

def load_catalog(catalog_file):
    """Load message catalog from JSON file"""
    try:
        with open(catalog_file, 'r', encoding='utf-8') as f:
            return json.load(f)
    except Exception as e:
        print(f"Error loading catalog: {e}", file=sys.stderr)
        sys.exit(1)

def parse_modeled_message(line):
    """
    Parse a modeled message log line.
    
    Format: [LEVEL] [CONTEXT] [MsgId:NNNN] param1=value1, param2=value2
    
    Returns: {
        'timestamp': '...',
        'level': 'INFO',
        'context': 'APP',
        'msg_id': '1000',
        'params': {'param1': 'value1', 'param2': 'value2'}
    }
    """
    # Pattern: [timestamp] [LEVEL] [CONTEXT] [MsgId:NNNN] params...
    pattern = r'\[([^\]]+)\]\s*\[([^\]]+)\]\s*\[([^\]]+)\]\s*\[MsgId:(\d+)\]\s*(.+)'
    match = re.search(pattern, line)
    
    if not match:
        return None
    
    timestamp = match.group(1)
    level = match.group(2)
    context = match.group(3)
    msg_id = match.group(4)
    params_str = match.group(5)
    
    # Parse parameters (key=value format)
    params = {}
    if params_str:
        # Split by comma, but respect quoted strings
        param_pattern = r'(\w+)=([^,]+?)(?:,\s*|$)'
        for match in re.finditer(param_pattern, params_str):
            key = match.group(1)
            value = match.group(2).strip().strip('"')
            params[key] = value
    
    return {
        'timestamp': timestamp,
        'level': level,
        'context': context,
        'msg_id': msg_id,
        'params': params,
        'raw': line.strip()
    }

def format_message(catalog, log_entry):
    """
    Format a log entry using the message catalog.
    
    Returns formatted message string or original line if not found.
    """
    msg_id = log_entry['msg_id']
    
    if msg_id not in catalog['messages']:
        return f"[Unknown MsgId:{msg_id}] {log_entry['params']}"
    
    msg_info = catalog['messages'][msg_id]
    name = msg_info['name']
    
    # Build formatted string
    params_str = ', '.join(f"{k}={v}" for k, v in log_entry['params'].items())
    
    return (f"[{log_entry['timestamp']}] [{log_entry['level']}] [{log_entry['context']}] "
            f"{name} (ID:{msg_id}): {params_str}")

def analyze_logs(catalog_file, log_file, options):
    """Analyze log file and decode modeled messages"""
    
    catalog = load_catalog(catalog_file)
    
    if options.get('show_catalog'):
        print("=== Message Catalog ===")
        print(f"Application: {catalog['application']}")
        print(f"Version: {catalog['version']}")
        print(f"Total messages: {catalog['total_messages']}")
        print()
    
    stats = defaultdict(int)
    
    try:
        with open(log_file, 'r', encoding='utf-8') as f:
            line_num = 0
            for line in f:
                line_num += 1
                
                # Try to parse as modeled message
                log_entry = parse_modeled_message(line)
                
                if log_entry:
                    stats[log_entry['msg_id']] += 1
                    
                    if not options.get('stats_only'):
                        # Format and print the message
                        formatted = format_message(catalog, log_entry)
                        print(formatted)
                else:
                    # Not a modeled message, print as-is if verbose
                    if options.get('show_all'):
                        print(line.rstrip())
    
    except Exception as e:
        print(f"Error reading log file: {e}", file=sys.stderr)
        sys.exit(1)
    
    # Print statistics
    if options.get('show_stats') or options.get('stats_only'):
        print("\n=== Message Statistics ===")
        print(f"{'Message ID':<12} {'Count':<10} {'Name':<30} {'Routing'}")
        print("-" * 80)
        
        for msg_id in sorted(stats.keys(), key=int):
            count = stats[msg_id]
            msg_info = catalog['messages'].get(msg_id, {})
            name = msg_info.get('name', 'Unknown')
            routing = msg_info.get('routing', 'Unknown')
            print(f"{msg_id:<12} {count:<10} {name:<30} {routing}")
        
        print(f"\nTotal modeled messages: {sum(stats.values())}")

def main():
    parser = argparse.ArgumentParser(
        description='Analyze logs containing modeled messages',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Basic usage
  %(prog)s message_catalog.json app.log
  
  # Show statistics
  %(prog)s message_catalog.json app.log --stats
  
  # Show only statistics (no log output)
  %(prog)s message_catalog.json app.log --stats-only
  
  # Show all lines (including non-modeled)
  %(prog)s message_catalog.json app.log --show-all
        """
    )
    
    parser.add_argument('catalog',
                       help='Message catalog JSON file')
    parser.add_argument('logfile',
                       help='Log file to analyze')
    parser.add_argument('-s', '--stats',
                       action='store_true',
                       help='Show statistics after output')
    parser.add_argument('--stats-only',
                       action='store_true',
                       help='Show only statistics (no log output)')
    parser.add_argument('-a', '--show-all',
                       action='store_true',
                       help='Show all log lines (including non-modeled)')
    parser.add_argument('-c', '--show-catalog',
                       action='store_true',
                       help='Show catalog information')
    
    args = parser.parse_args()
    
    options = {
        'show_stats': args.stats,
        'stats_only': args.stats_only,
        'show_all': args.show_all,
        'show_catalog': args.show_catalog
    }
    
    analyze_logs(args.catalog, args.logfile, options)

if __name__ == '__main__':
    main()
