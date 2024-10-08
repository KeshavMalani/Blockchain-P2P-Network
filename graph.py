from graphviz import Digraph

# Initialize the graph
dot = Digraph(comment='Blockchain', format='png')

# Define colors for each peer
colors = {'0': 'red', '1': 'blue', '2': 'green', '3': 'yellow'}

# Genesis block
genesis_hash = '0x0000'
dot.node(genesis_hash, 'Genesis', shape='ellipse', style='filled', color='gray')

# Blocks by peers
blocks = {
    '0': [('0xb5a6', genesis_hash), ('0x8d09', '0x9e1c'), ('0x1caf', '0x8d09'), ('0xb309', '0x1caf'), ('0x9e91', '0xb309'), 
          ('0xafe5', '0x9e91'), ('0xbb2a', '0xafe5'), ('0x7a02', '0xbb2a'), ('0x69a6', '0x7a02'), ('0x3ace', '0x69a6'),
          ('0x69c2', '0x3ace'), ('0x4150', '0x69c2'), ('0x1ab0', '0x4150'), ('0x23ce', '0x1ab0'), ('0x1387', '0x23ce')],
    '1': [('0xe12d', genesis_hash), ('0x314e', '0x9e1c'), ('0x971a', '0x314e'), ('0x573f', '0x971a')],
    '2': [('0x9d69', genesis_hash), ('0xda92', '0x9e1c'), ('0x5bd8', '0xda92'), ('0x1093', '0x5bd8'), ('0xea0d', '0x1093'), 
          ('0x233f', '0xea0d')],
    '3': [('0x6b26', genesis_hash)]
}

# Add nodes and edges for each peer
for peer, blks in blocks.items():
    for block_hash, prev_hash in blks:
        dot.node(block_hash, f'{block_hash}\n(Peer {peer})', style='filled', color=colors[peer])
        dot.edge(prev_hash, block_hash, color=colors[peer])

# Render and save the graph
dot.render('blockchain_tree')

# To display in Jupyter, you can use:
# dot
