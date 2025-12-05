#!/usr/bin/env python3
"""
mysh_llm.py - AI Assistant for PicoBox Shell

This script provides natural language to shell command translation using:
1. RAG (Retrieval-Augmented Generation) - scoring commands by relevance
2. LLM (OpenAI API) - generating smart command suggestions
3. Fallback heuristics - works without API key

Usage:
    python3 mysh_llm.py "show me all files"

Environment Variables:
    OPENAI_API_KEY     - OpenAI API key (optional, uses fallback if not set)
    AI_SHELL           - Alternative API key (same as cmd_ai.c uses)
    MYSH_PATH          - Path to picobox binary (default: picobox)
    MYSH_CATALOG_CMD   - Command to get catalog (default: ./build/picobox --commands-json)
    MYSH_LLM_MODEL     - OpenAI model (default: gpt-4o-mini)
    MYSH_LLM_DEBUG     - Enable debug output (set to 1)
"""

import os
import sys
import json
import subprocess
from typing import List, Optional, Dict, Any
from dataclasses import dataclass


@dataclass
class CommandInfo:
    """Information about a shell command"""
    name: str
    summary: str
    description: str
    usage: str


def debug_print(msg: str) -> None:
    """Print debug message to stderr if debug mode enabled"""
    if os.environ.get('MYSH_LLM_DEBUG') == '1':
        print(f"[mysh_llm] {msg}", file=sys.stderr)


def run_catalog_command() -> Optional[str]:
    """
    Run the shell's --commands-json command to get command catalog

    Returns JSON string or None on error
    """
    catalog_cmd = os.environ.get('MYSH_CATALOG_CMD', './build/picobox --commands-json')
    debug_print(f"Running catalog command: {catalog_cmd}")

    try:
        result = subprocess.run(
            catalog_cmd.split(),
            capture_output=True,
            text=True,
            timeout=5
        )

        if result.returncode == 0:
            return result.stdout
        else:
            debug_print(f"Catalog command failed: {result.stderr}")
            return None
    except Exception as e:
        debug_print(f"Error running catalog command: {e}")
        return None


def load_catalog_from_file(filename: str = "commands.json") -> Optional[str]:
    """
    Load command catalog from a JSON file (fallback method)

    Returns JSON string or None if file doesn't exist
    """
    try:
        with open(filename, 'r') as f:
            return f.read()
    except FileNotFoundError:
        return None


def load_command_catalog() -> List[CommandInfo]:
    """
    Load the command catalog using available methods

    Tries (in order):
    1. Run catalog command (picobox --commands-json)
    2. Load from commands.json file
    3. Return empty list

    Returns list of CommandInfo objects
    """
    # Try running catalog command
    json_str = run_catalog_command()

    # Fallback to file
    if not json_str:
        debug_print("Falling back to commands.json file")
        json_str = load_catalog_from_file()

    # Parse JSON
    if not json_str:
        debug_print("No catalog available")
        return []

    try:
        data = json.loads(json_str)
        commands = []

        for cmd in data.get('commands', []):
            commands.append(CommandInfo(
                name=cmd.get('name', ''),
                summary=cmd.get('summary', ''),
                description=cmd.get('description', ''),
                usage=cmd.get('usage', '')
            ))

        debug_print(f"Loaded {len(commands)} commands")
        return commands

    except json.JSONDecodeError as e:
        debug_print(f"JSON parse error: {e}")
        return []


def score_command(query: str, cmd: CommandInfo) -> int:
    """
    Score a command's relevance to the query (simple keyword matching)

    Scoring:
    - Name match: 3 points per word
    - Summary match: 2 points per word
    - Description match: 1 point per word

    Returns integer score (higher = more relevant)
    """
    query_lower = query.lower()
    query_words = query_lower.split()

    score = 0

    # Score name matches (highest weight)
    for word in query_words:
        if word in cmd.name.lower():
            score += 3

    # Score summary matches (medium weight)
    summary_lower = cmd.summary.lower()
    for word in query_words:
        if word in summary_lower:
            score += 2

    # Score description matches (low weight)
    desc_lower = cmd.description.lower()
    for word in query_words:
        if word in desc_lower:
            score += 1

    return score


def select_relevant_commands(query: str, catalog: List[CommandInfo], k: int = 5) -> List[CommandInfo]:
    """
    Select the top k most relevant commands for the query (RAG retrieval)

    Returns list of CommandInfo objects, sorted by relevance
    """
    # Score all commands
    scored = [(cmd, score_command(query, cmd)) for cmd in catalog]

    # Sort by score (descending)
    scored.sort(key=lambda x: x[1], reverse=True)

    # Return top k
    return [cmd for cmd, score in scored[:k]]


def build_prompt(query: str, catalog: List[CommandInfo]) -> str:
    """
    Build the prompt for the LLM

    Includes:
    - System instructions
    - Relevant command documentation (RAG context)
    - User query

    Returns formatted prompt string
    """
    # Get top 5 relevant commands
    relevant = select_relevant_commands(query, catalog, k=5)

    # Build command documentation
    cmd_docs = []
    for cmd in relevant:
        doc = f"Command: {cmd.name}\n"
        doc += f"Summary: {cmd.summary}\n"
        if cmd.description:
            doc += f"Description: {cmd.description}\n"
        doc += f"Usage: {cmd.usage}\n"
        cmd_docs.append(doc)

    context = "\n".join(cmd_docs)

    prompt = f"""You are a Unix shell command expert. Given the user's request, suggest the most appropriate command.

Available Commands:
{context}

User Request: {query}

Respond with ONLY the shell command, no explanation. The command should:
- Use commands from the list above
- Be a valid single-line command
- Use proper syntax (pipes, redirects allowed)
- Be executable as-is

Command:"""

    return prompt


def call_llm(prompt: str) -> Optional[str]:
    """
    Call OpenAI API to get command suggestion

    Returns suggested command string or None on error/not configured
    """
    api_key = os.environ.get('OPENAI_API_KEY') or os.environ.get('AI_SHELL')

    if not api_key:
        debug_print("No OpenAI API key found (OPENAI_API_KEY or AI_SHELL)")
        return None

    model = os.environ.get('MYSH_LLM_MODEL', 'gpt-4o-mini')

    try:
        # Use openai library if available
        try:
            import openai
            client = openai.OpenAI(api_key=api_key)

            response = client.chat.completions.create(
                model=model,
                messages=[
                    {"role": "system", "content": "You are a Unix shell command expert. Respond with only the command, no explanation."},
                    {"role": "user", "content": prompt}
                ],
                temperature=0.3,
                max_tokens=100
            )

            suggestion = response.choices[0].message.content.strip()
            debug_print(f"LLM suggestion: {suggestion}")
            return suggestion

        except ImportError:
            # Fallback to curl
            debug_print("openai library not found, using curl")

            import tempfile
            with tempfile.NamedTemporaryFile(mode='w', suffix='.json', delete=False) as f:
                json.dump({
                    "model": model,
                    "messages": [
                        {"role": "system", "content": "You are a Unix shell command expert. Respond with only the command, no explanation."},
                        {"role": "user", "content": prompt}
                    ],
                    "temperature": 0.3,
                    "max_tokens": 100
                }, f)
                request_file = f.name

            try:
                result = subprocess.run([
                    'curl',
                    '-s',
                    'https://api.openai.com/v1/chat/completions',
                    '-H', f'Authorization: Bearer {api_key}',
                    '-H', 'Content-Type: application/json',
                    '-d', f'@{request_file}'
                ], capture_output=True, text=True, timeout=30)

                if result.returncode == 0:
                    data = json.loads(result.stdout)
                    if 'choices' in data:
                        suggestion = data['choices'][0]['message']['content'].strip()
                        debug_print(f"LLM suggestion: {suggestion}")
                        return suggestion
                    elif 'error' in data:
                        debug_print(f"API error: {data['error']}")
                        return None
            finally:
                os.unlink(request_file)

    except Exception as e:
        debug_print(f"LLM call failed: {e}")
        return None


def heuristic_suggestion(query: str, catalog: List[CommandInfo]) -> str:
    """
    Generate a basic command suggestion using heuristics (no LLM)

    This is the fallback when OpenAI API is not available.
    Uses simple keyword matching to pick the most relevant command.

    Returns command string
    """
    if not catalog:
        return "echo 'No commands available'"

    # Get most relevant command
    relevant = select_relevant_commands(query, catalog, k=1)

    if relevant:
        cmd = relevant[0]

        # Simple heuristics based on query
        query_lower = query.lower()

        # List/show files
        if any(word in query_lower for word in ['list', 'show', 'files', 'directory']):
            if 'all' in query_lower or 'hidden' in query_lower:
                return 'ls -la'
            return 'ls'

        # Find
        if 'find' in query_lower:
            return 'find .'

        # Count/lines
        if 'count' in query_lower or 'lines' in query_lower:
            return 'wc -l'

        # Search/grep
        if 'search' in query_lower or 'grep' in query_lower:
            return 'grep'

        # Default: just return the command name
        return cmd.name

    return 'help'


def suggest_command(query: str) -> str:
    """
    Main function: suggest a command for the given query

    Process:
    1. Load command catalog
    2. Try LLM (if API key available)
    3. Fall back to heuristic

    Returns suggested command string
    """
    # Load catalog
    catalog = load_command_catalog()

    # Build prompt
    prompt = build_prompt(query, catalog)

    # Try LLM first
    suggestion = call_llm(prompt)

    # Fall back to heuristic
    if not suggestion:
        debug_print("Using heuristic fallback")
        suggestion = heuristic_suggestion(query, catalog)

    return suggestion


def main(argv: List[str]) -> int:
    """
    Entry point

    Usage: mysh_llm.py <query>
    Prints suggestion to stdout (single line, no newline at end)

    Returns 0 on success, 1 on error
    """
    if len(argv) < 2:
        print("Usage: mysh_llm.py <natural language query>", file=sys.stderr)
        return 1

    query = ' '.join(argv[1:])

    if not query.strip():
        print("echo 'LLM helper: empty query'")
        return 0

    suggestion = suggest_command(query)

    # Print suggestion (no trailing newline for shell to consume)
    print(suggestion, end='')

    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
