module C17 ( G1GAT_0_gat, G2GAT_1_gat, G3GAT_2_gat, G6GAT_3_gat, G7GAT_4_gat, G22GAT_10_gat, G23GAT_9_gat);

input G1GAT_0_gat;
input G2GAT_1_gat;
input G3GAT_2_gat;
input G6GAT_3_gat;
input G7GAT_4_gat;

output G22GAT_10_gat;
output G23GAT_9_gat;

NAND2XL U_g1 (.A(G6GAT_3_gat), .B(G3GAT_2_gat), .Y(G11GAT_5_gat) );
NAND2XL U_g2 (.A(G3GAT_2_gat), .B(G1GAT_0_gat), .Y(G10GAT_6_gat) );
NAND2XL U_g3 (.A(G7GAT_4_gat), .B(G11GAT_5_gat), .Y(G19GAT_7_gat) );
NAND2XL U_g4 (.A(G11GAT_5_gat), .B(G2GAT_1_gat), .Y(G16GAT_8_gat) );
NAND2XL U_g5 (.A(G19GAT_7_gat), .B(G16GAT_8_gat), .Y(G23GAT_9_gat) );
NAND2XL U_g6 (.A(G16GAT_8_gat), .B(G10GAT_6_gat), .Y(G22GAT_10_gat) );

endmodule
